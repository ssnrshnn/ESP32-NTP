#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <time.h>
#include <WiFiUdp.h>

// LED Pin Definitions
#define WIFI_LED_PIN 32  // First LED
#define NTP_LED_PIN 33   // Second LED

// LED Pattern timing (all in milliseconds)
#define BLINK_ON_TIME 65     // Time LED stays on for each blink
#define BLINK_OFF_TIME 65    // Time LED stays off between blinks
#define LED_PAUSE_TIME 650    // Pause between LED sequences
#define CYCLE_PAUSE_TIME 1300 // Pause before repeating the whole cycle

// LED Pattern variables
unsigned long lastLedUpdate = 0;
int patternStep = 0;  // Keeps track of where we are in the pattern

// TFT Display setup
TFT_eSPI tft = TFT_eSPI();

// WiFi credentials
const char* ssid = "SSID";
const char* password = "PASSWORD";

// NTP Servers
const char* ntpServer1 = "NTP_SERVER_1";
const char* ntpServer2 = "NTP_SERVER_2";
const char* ntpServer3 = "NTP_SERVER_3";

const long  gmtOffset_sec = 3600;  // GMT+1
const int   daylightOffset_sec = 3600; // Daylight saving

// NTP Server setup
WiFiUDP udp;
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];
unsigned int localPort = 123; // NTP port

// --- Modern Sleek Design Colors ---
#define DARK_BACKGROUND 0x1082 // Deep navy blue
#define SURFACE_COLOR 0x2945 // Darker blue-grey surface
#define ACCENT_PRIMARY 0x05FF // Electric blue
#define ACCENT_SECONDARY 0xF7BE // Warm orange
#define SUCCESS_COLOR 0x4FE6 // Modern green
#define WARNING_COLOR 0xFE60 // Amber
#define ERROR_COLOR 0xF8B2 // Coral red
#define TEXT_PRIMARY 0xFFFF // Pure white
#define TEXT_SECONDARY 0xAD55 // Light blue-grey
#define TEXT_MUTED 0x6B4D // Medium grey
#define HIGHLIGHT_COLOR 0x867F // Purple accent
#define SHADOW_COLOR 0x0841 // Very dark blue for depth

// WiFi signal strength colors
#define WIFI_EXCELLENT SUCCESS_COLOR
#define WIFI_GOOD ACCENT_SECONDARY
#define WIFI_FAIR WARNING_COLOR
#define WIFI_POOR ERROR_COLOR
#define WIFI_DISCONNECTED TEXT_MUTED

// German flag colors - RGB565 format with maximum saturation
#define FLAG_BLACK 0x0000                    // RGB(0,0,0)
#define FLAG_RED 0xF800                      // Pure red: RGB(255,0,0) - maximum red, no blue
#define FLAG_GOLD 0xFFE0                     // Pure yellow: RGB(255,255,0) - full red and green, no blue

// Client info colors
#define CLIENT_HEADER_COLOR ACCENT_SECONDARY
#define CLIENT_TEXT_COLOR TEXT_PRIMARY
#define NTP_SERVER_COLOR ACCENT_PRIMARY

// Screen dimensions - portrait for ST7789V 240x320
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// Update intervals
const unsigned long NTP_SYNC_INTERVAL = 30000; // Sync with NTP server every 30 seconds
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000; // Update display locally every 1 second

unsigned long lastNtpSync = 0;
unsigned long lastDisplayUpdate = 0;

struct tm currentTimeInfo; // Stores the current time, updated locally and by NTP
bool ntpTimeAvailable = false; // Flag to indicate if NTP time has been fetched

void setup() {
  Serial.begin(115200);
  
  // Initialize LED pins
  pinMode(WIFI_LED_PIN, OUTPUT);
  pinMode(NTP_LED_PIN, OUTPUT);
  
  // Start with both LEDs off
  digitalWrite(WIFI_LED_PIN, LOW);
  digitalWrite(NTP_LED_PIN, LOW);
  
  tft.init();
  tft.setRotation(0); // Portrait mode
  tft.fillScreen(DARK_BACKGROUND);
  tft.setTextDatum(MC_DATUM);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print(".");
    // Blink WiFi LED while connecting
    digitalWrite(WIFI_LED_PIN, !digitalRead(WIFI_LED_PIN));
  }
  digitalWrite(WIFI_LED_PIN, HIGH);  // WiFi LED solid ON when connected
  Serial.println("\nWiFi connected.");
  
  // Start NTP server
  udp.begin(localPort);
  Serial.printf("NTP Server started on port %d\n", localPort);
  
  Serial.println("Configuring time from NTP...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2, ntpServer3);
  
  struct tm tempTimeInfo;
  if(!getLocalTime(&tempTimeInfo)){
    Serial.println("Failed to obtain initial NTP time.");
    digitalWrite(NTP_LED_PIN, LOW);  // NTP LED off if sync fails
  } else {
    Serial.println("Initial NTP time obtained.");
    currentTimeInfo = tempTimeInfo;
    ntpTimeAvailable = true;
    digitalWrite(NTP_LED_PIN, HIGH);  // NTP LED on when synced
  }
  
  drawLayout();
  
  if (ntpTimeAvailable) {
    renderDisplay();
    lastDisplayUpdate = millis(); 
  } else {
    tft.setTextColor(TFT_RED, DARK_BACKGROUND);
    tft.drawString("Waiting for NTP...", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 2);
  }
  lastNtpSync = millis(); 
}

void updateLEDPattern() {
  unsigned long currentMillis = millis();
  
  switch(patternStep) {
    case 0:  // First LED first blink ON
      digitalWrite(WIFI_LED_PIN, HIGH);
      digitalWrite(NTP_LED_PIN, LOW);
      patternStep++;
      lastLedUpdate = currentMillis;
      break;
      
    case 1:  // First LED first blink OFF
      if(currentMillis - lastLedUpdate >= BLINK_ON_TIME) {
        digitalWrite(WIFI_LED_PIN, LOW);
        patternStep++;
        lastLedUpdate = currentMillis;
      }
      break;
      
    case 2:  // First LED second blink ON
      if(currentMillis - lastLedUpdate >= BLINK_OFF_TIME) {
        digitalWrite(WIFI_LED_PIN, HIGH);
        patternStep++;
        lastLedUpdate = currentMillis;
      }
      break;
      
    case 3:  // First LED second blink OFF
      if(currentMillis - lastLedUpdate >= BLINK_ON_TIME) {
        digitalWrite(WIFI_LED_PIN, LOW);
        patternStep++;
        lastLedUpdate = currentMillis;
      }
      break;
      
    case 4:  // Pause between LEDs
      if(currentMillis - lastLedUpdate >= LED_PAUSE_TIME) {
        patternStep++;
        lastLedUpdate = currentMillis;
      }
      break;
      
    case 5:  // Second LED first blink ON
      digitalWrite(NTP_LED_PIN, HIGH);
      patternStep++;
      lastLedUpdate = currentMillis;
      break;
      
    case 6:  // Second LED first blink OFF
      if(currentMillis - lastLedUpdate >= BLINK_ON_TIME) {
        digitalWrite(NTP_LED_PIN, LOW);
        patternStep++;
        lastLedUpdate = currentMillis;
      }
      break;
      
    case 7:  // Second LED second blink ON
      if(currentMillis - lastLedUpdate >= BLINK_OFF_TIME) {
        digitalWrite(NTP_LED_PIN, HIGH);
        patternStep++;
        lastLedUpdate = currentMillis;
      }
      break;
      
    case 8:  // Second LED second blink OFF
      if(currentMillis - lastLedUpdate >= BLINK_ON_TIME) {
        digitalWrite(NTP_LED_PIN, LOW);
        patternStep++;
        lastLedUpdate = currentMillis;
      }
      break;
      
    case 9:  // Final pause before repeating
      if(currentMillis - lastLedUpdate >= CYCLE_PAUSE_TIME) {
        patternStep = 0;  // Reset to start
        lastLedUpdate = currentMillis;
      }
      break;
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // Update LED pattern
  updateLEDPattern();

  // Handle NTP server requests
  handleNTPServer();

  if (currentMillis - lastNtpSync >= NTP_SYNC_INTERVAL) {
    Serial.println("Attempting NTP resync...");
    struct tm tempTimeInfo;
    if(getLocalTime(&tempTimeInfo)){ 
      Serial.println("NTP time resynced.");
      currentTimeInfo = tempTimeInfo;
      ntpTimeAvailable = true;
      renderDisplay(); 
      lastDisplayUpdate = currentMillis; 
    } else {
      Serial.println("Failed to resync NTP time. Will keep ticking locally.");
    }
    lastNtpSync = currentMillis;
  }

  // Check WiFi status
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
  }

  if (ntpTimeAvailable && (currentMillis - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL)) {
    currentTimeInfo.tm_sec++; 
    time_t epochTime = mktime(&currentTimeInfo); 
    currentTimeInfo = *localtime(&epochTime);   
    
    renderDisplay(); 
    lastDisplayUpdate = currentMillis;
  }
}

void handleNTPServer() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    Serial.println("NTP request received");
    
    // Read the packet
    udp.read(packetBuffer, NTP_PACKET_SIZE);
    
    // Clear the buffer
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    
    // Set NTP response fields
    packetBuffer[0] = 0b00100100; // LI, Version, Mode
    packetBuffer[1] = 0;          // Stratum
    packetBuffer[2] = 6;          // Polling interval
    packetBuffer[3] = 0xEC;       // Peer Clock Precision
    
    // Get current time as NTP timestamp
    time_t now = time(nullptr);
    unsigned long ntpTime = now + 2208988800UL; // Convert to NTP time
    
    // Transmit timestamp
    packetBuffer[40] = (ntpTime >> 24) & 0xFF;
    packetBuffer[41] = (ntpTime >> 16) & 0xFF;
    packetBuffer[42] = (ntpTime >> 8) & 0xFF;
    packetBuffer[43] = ntpTime & 0xFF;
    
    // Send response
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(packetBuffer, NTP_PACKET_SIZE);
    udp.endPacket();
  }
}

uint16_t getWiFiSignalColor() {
  if (WiFi.status() != WL_CONNECTED) {
    return WIFI_DISCONNECTED;
  }
  
  int32_t rssi = WiFi.RSSI();
  
  if (rssi >= -50) {
    return WIFI_EXCELLENT;
  } else if (rssi >= -60) {
    return WIFI_GOOD;
  } else if (rssi >= -70) {
    return WIFI_FAIR;
  } else {
    return WIFI_POOR;
  }
}

void drawWiFiIcon() {
  int iconX = SCREEN_WIDTH - 30; // Position from right edge
  int iconY = 15; // Top position
  
  uint16_t signalColor = getWiFiSignalColor();
  
  // Clear the WiFi icon area first
  tft.fillRect(iconX - 5, iconY - 5, 25, 20, DARK_BACKGROUND);
  
  if (WiFi.status() != WL_CONNECTED) {
    // Draw disconnected icon (X mark)
    tft.drawLine(iconX, iconY, iconX + 12, iconY + 12, WIFI_DISCONNECTED);
    tft.drawLine(iconX + 12, iconY, iconX, iconY + 12, WIFI_DISCONNECTED);
    return;
  }
  
  // Draw WiFi signal bars based on signal strength
  int32_t rssi = WiFi.RSSI();
  int bars = 0;
  
  if (rssi >= -50) bars = 4;      // Excellent
  else if (rssi >= -60) bars = 3; // Good  
  else if (rssi >= -70) bars = 2; // Fair
  else bars = 1;                  // Poor
  
  // Draw signal bars (right to left, increasing height)
  for (int i = 0; i < 4; i++) {
    int barX = iconX + (i * 3);
    int barHeight = (i + 1) * 2 + 2;
    int barY = iconY + 10 - barHeight;
    
    if (i < bars) {
      // Active bar - use signal color
      tft.fillRect(barX, barY, 2, barHeight, signalColor);
    } else {
      // Inactive bar - use muted color
      tft.fillRect(barX, barY, 2, barHeight, TEXT_MUTED);
    }
  }
  
  // Draw small base arc for WiFi symbol
  tft.drawCircle(iconX + 6, iconY + 12, 2, signalColor);
  tft.fillCircle(iconX + 6, iconY + 12, 1, signalColor);
}

void drawGermanFlag() {
  // Draw "DE" text in white color
  tft.setTextColor(TEXT_PRIMARY, DARK_BACKGROUND);
  tft.drawString("DE", 25, 22, 2);  // Using same y-position as "NTP SERVER" text
}

void drawLayout() {
  tft.fillScreen(DARK_BACKGROUND);
  
  // Top header bar with gradient effect
  tft.fillRoundRect(5, 5, SCREEN_WIDTH - 10, 35, 8, SURFACE_COLOR);
  tft.fillRoundRect(6, 6, SCREEN_WIDTH - 12, 33, 7, DARK_BACKGROUND);
  tft.drawRoundRect(5, 5, SCREEN_WIDTH - 10, 35, 8, ACCENT_PRIMARY);
  
  // Header elements
  drawGermanFlag();
  tft.setTextColor(TEXT_PRIMARY, DARK_BACKGROUND);
  tft.drawString("NTP SERVER", SCREEN_WIDTH/2, 22, 2);
  drawWiFiIcon();
  
  // Main time/date card with modern styling
  tft.fillRoundRect(10, 50, SCREEN_WIDTH - 20, 120, 12, SHADOW_COLOR);
  tft.fillRoundRect(8, 48, SCREEN_WIDTH - 16, 120, 12, SURFACE_COLOR);
  tft.drawRoundRect(8, 48, SCREEN_WIDTH - 16, 120, 12, ACCENT_PRIMARY);
  
  // Status indicators card
  tft.fillRoundRect(10, 180, SCREEN_WIDTH - 20, 40, 8, SHADOW_COLOR);
  tft.fillRoundRect(8, 178, SCREEN_WIDTH - 16, 40, 8, SURFACE_COLOR);
  tft.drawRoundRect(8, 178, SCREEN_WIDTH - 16, 40, 8, SUCCESS_COLOR);
  
  // Client info card
  tft.fillRoundRect(10, 230, SCREEN_WIDTH - 20, 45, 8, SHADOW_COLOR);
  tft.fillRoundRect(8, 228, SCREEN_WIDTH - 16, 45, 8, SURFACE_COLOR);
  tft.drawRoundRect(8, 228, SCREEN_WIDTH - 16, 45, 8, ACCENT_SECONDARY);
  
  // Bottom status strip
  tft.fillRoundRect(5, 285, SCREEN_WIDTH - 10, 25, 6, SHADOW_COLOR);
  tft.fillRoundRect(3, 283, SCREEN_WIDTH - 6, 25, 6, SURFACE_COLOR);
  tft.drawRoundRect(3, 283, SCREEN_WIDTH - 6, 25, 6, HIGHLIGHT_COLOR);
}

void renderDisplay() {
  if (!ntpTimeAvailable) { return; }

  // Clear main card content
  tft.fillRoundRect(10, 50, SCREEN_WIDTH - 22, 116, 10, SURFACE_COLOR);
  
  // Update WiFi icon
  drawWiFiIcon();
  
  // Date display with modern typography
  char dateStr[20];
  strftime(dateStr, sizeof(dateStr), "%d %B %Y", &currentTimeInfo);
  tft.setTextColor(ACCENT_SECONDARY, SURFACE_COLOR);
  tft.drawString(dateStr, SCREEN_WIDTH/2, 70, 4);
  
  // Elegant separator
  tft.fillRoundRect(30, 85, SCREEN_WIDTH - 60, 2, 1, TEXT_MUTED);
  
  // Day of week with accent
  char dayOfWeekStr[10];
  strftime(dayOfWeekStr, sizeof(dayOfWeekStr), "%A", &currentTimeInfo);
  tft.setTextColor(SUCCESS_COLOR, SURFACE_COLOR);
  tft.drawString(dayOfWeekStr, SCREEN_WIDTH/2, 100, 2);
  
  // Time display - large and prominent
  char timeStr[9];
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &currentTimeInfo);
  tft.setTextColor(ACCENT_PRIMARY, SURFACE_COLOR);
  tft.drawString(timeStr, SCREEN_WIDTH/2, 138, 7);
  
  // Status indicators with modern design
  tft.fillRoundRect(10, 180, SCREEN_WIDTH - 22, 36, 6, SURFACE_COLOR);
  
  // Status dots and labels
  tft.setTextColor(TEXT_SECONDARY, SURFACE_COLOR);
  tft.drawString("SYNC", 40, 190, 1);
  tft.fillCircle(60, 190, 4, SUCCESS_COLOR);
  tft.fillCircle(60, 190, 2, TEXT_PRIMARY);
  
  tft.drawString("NTP", 90, 190, 1);
  tft.fillCircle(110, 190, 4, SUCCESS_COLOR);
  tft.fillCircle(110, 190, 2, TEXT_PRIMARY);
  
  tft.drawString("SERVER", 140, 190, 1);
  tft.fillCircle(180, 190, 4, SUCCESS_COLOR);
  tft.fillCircle(180, 190, 2, TEXT_PRIMARY);
  
  tft.setTextColor(TEXT_MUTED, SURFACE_COLOR);
  tft.drawString("ALL SYSTEMS OPERATIONAL", SCREEN_WIDTH/2, 205, 1);
  
  // Client panel with modern styling
  tft.fillRoundRect(10, 230, SCREEN_WIDTH - 22, 41, 6, SURFACE_COLOR);
  tft.setTextColor(CLIENT_HEADER_COLOR, SURFACE_COLOR);
  tft.drawString("CONNECTED CLIENTS", SCREEN_WIDTH/2, 245, 2);
  
  // Client count with badge style
  tft.fillRoundRect(SCREEN_WIDTH/2 - 25, 255, 50, 12, 6, ACCENT_SECONDARY);
  tft.setTextColor(DARK_BACKGROUND, ACCENT_SECONDARY);
  tft.drawString("0 ACTIVE", SCREEN_WIDTH/2, 261, 1);
  
  // Bottom status bar - NTP Server IP with modern styling
  tft.fillRoundRect(5, 285, SCREEN_WIDTH - 12, 21, 4, SURFACE_COLOR);
  tft.setTextColor(NTP_SERVER_COLOR, SURFACE_COLOR);
  String ipStr = "NTP: " + WiFi.localIP().toString();
  tft.drawString(ipStr.c_str(), SCREEN_WIDTH/2, 295, 2);
}
