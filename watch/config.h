#ifndef CONFIG_H
#define CONFIG_H

#define LED 12
#define BUTTON 19

#define SCREEN_MOSI 23 // In some display driver board, it might be written as "SDA" and so on.
#define SCREEN_SCLK 18
#define SCREEN_CS   2  // Chip select control pin
#define SCREEN_DC   15  // Data Command control pin
#define SCREEN_RST  4  // Reset pin (could connect to Arduino RESET pin)

#endif
