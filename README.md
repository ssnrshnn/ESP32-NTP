 # ESP32 NTP Server with TFT Display

A modern NTP server implementation using ESP32 and TFT display, featuring a sleek UI design and LED status indicators.

## Hardware Requirements

- ESP32 Development Board
- 2.0" TFT SPI Display (GMT020-02 VER:1.1)
  - Display Driver: ST7789V
  - Resolution: 240x320
- 2x LEDs (for status indicators)
- 2x 220Ω resistors
- Jumper wires

## Pin Connections

### TFT Display
| TFT Pin | ESP32 Pin |
|---------|-----------|
| CS      | GPIO5     |
| DC      | GPIO4     |
| RST     | GPIO22    |
| SDA(MOSI)| GPIO23    |
| SCL(SCK) | GPIO18    |
| VCC     | 3.3V      |
| GND     | GND       |

### Status LEDs
| Component | Connection |
|-----------|------------|
| WiFi LED  | GPIO32 → LED → 220Ω → GND |
| NTP LED   | GPIO33 → LED → 220Ω → GND |

## Features

- NTP Server functionality using German NTP pool
- Real-time clock synchronization
- Modern UI with:
  - Digital clock display
  - Date and day display
  - WiFi status indicator
  - System status indicators
  - Connected clients counter
  - Server IP display
- Status LED indicators:
  - WiFi connection status (blinking during connection, solid when connected)
  - NTP synchronization status

## Network Configuration

- WiFi SSID: "MARVEL"
- NTP Servers:
  - Primary: 0.de.pool.ntp.org
  - Secondary: 1.de.pool.ntp.org
  - Tertiary: 2.de.pool.ntp.org
- Time Zone: GMT+1 with DST support

## LED Patterns

The status LEDs follow this pattern:
1. First LED (WiFi):
   - Double blink (65ms on/off)
   - 650ms pause
2. Second LED (NTP):
   - Double blink (65ms on/off)
   - 1300ms pause before cycle repeats

## Display Layout

```
┌──────────────────────────┐
│ DE    NTP SERVER     WiFi│
├──────────────────────────┤
│      01 June 2025        │
│         Sunday           │
│       05:39:29          │
├──────────────────────────┤
│      SYNC • NTP • SERVER │
│  ALL SYSTEMS OPERATIONAL │
├──────────────────────────┤
│    CONNECTED CLIENTS     │
│         0 ACTIVE         │
├──────────────────────────┤
│   NTP: 192.168.1.183    │
└──────────────────────────┘
```

## Dependencies

- TFT_eSPI
- SPI
- WiFi
- WiFiUdp
- time.h

## Installation

1. Install the required libraries in Arduino IDE
2. Configure TFT_eSPI library for your display
3. Update WiFi credentials if needed
4. Upload the sketch to your ESP32

## Usage

1. Power up the ESP32
2. The device will automatically:
   - Connect to WiFi
   - Sync with NTP servers
   - Start its own NTP server
   - Display current status
3. Other devices on the network can use this device's IP as their NTP server

## Troubleshooting

- If WiFi LED keeps blinking: Check WiFi credentials
- If NTP LED is off: Check internet connectivity
- If display shows incorrect time: Verify timezone settings
- If clients can't connect: Check firewall settings for UDP port 123

## Credits

Created by ssnrshnn