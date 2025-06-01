#define USER_SETUP_INFO "User_Setup"

#define ST7789_DRIVER     // ST7789 driver

// Define the ESP32 pins used for SPI communication
#define TFT_MOSI 23   // Default ESP32 MOSI
#define TFT_SCLK 18   // Default ESP32 SCK
#define TFT_CS    5   // Chip select control pin
#define TFT_DC    4   // Data Command control pin
#define TFT_RST   22  // Reset pin

// For ST7789 displays with 240 x 320 resolution
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// Define the fonts that are to be used
#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font
#define LOAD_FONT2  // Font 2. Small 16 pixel high font
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font
#define LOAD_FONT6  // Font 6. Large 48 pixel font
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font
#define LOAD_GFXFF  // FreeFonts. Include access to the Adafruit_GFX free fonts FF1 to FF48

#define SMOOTH_FONT

// Display rotation
#define TFT_ROTATION 1  // Landscape mode

// Color definitions
#define TFT_BACKLIGHT_ON HIGH 