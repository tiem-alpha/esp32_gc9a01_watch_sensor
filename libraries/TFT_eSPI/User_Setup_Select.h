// This header file contains a list of user setup files and defines which one the
// compiler uses when the IDE performs a Verify/Compile or Upload.
//
// Users can create configurations for different boards and TFT displays.
// This makes selecting between hardware setups easy by "uncommenting" one line.

// The advantage of this hardware configuration method is that the examples provided
// with the library should work with immediately without any other changes being
// needed. It also improves the portability of users sketches to other hardware
// configurations and compatible libraries.
//
// Create a shortcut to this file on your desktop to permit quick access for editing.
// Re-compile and upload after making and saving any changes to this file.

// Example User_Setup files are stored in the "User_Setups" folder. These can be used
// unmodified or adapted for a particular hardware configuration.

#ifndef USER_SETUP_LOADED //  Lets PlatformIO users define settings in
                          //  platformio.ini, see notes in "Tools" folder.

///////////////////////////////////////////////////////
//   User configuration selection lines are below    //
///////////////////////////////////////////////////////

// Only ONE line below should be uncommented to define your setup.  Add extra lines and files as needed.

#include <User_Setup.h>           // Default setup is root library folder

#endif // USER_SETUP_LOADED



// Identical looking TFT displays may have a different colour ordering in the 16-bit colour
#define TFT_BGR 0   // Colour order Blue-Green-Red
#define TFT_RGB 1   // Colour order Red-Green-Blue



// Invoke 18-bit colour for selected displays
#if !defined (RPI_DISPLAY_TYPE) && !defined (TFT_PARALLEL_8_BIT) && !defined (TFT_PARALLEL_16_BIT) && !defined (ESP32_PARALLEL)
  #if defined (ILI9481_DRIVER) || defined (ILI9486_DRIVER) || defined (ILI9488_DRIVER)
    #define SPI_18BIT_DRIVER
  #endif
#endif

#include "TFT_Drivers/GC9A01_Defines.h"
   

// These are the pins for ESP8266 boards
//      Name   GPIO    NodeMCU      Function
#define PIN_D0  16  // GPIO16       WAKE
#define PIN_D1   5  // GPIO5        User purpose
#define PIN_D2   4  // GPIO4        User purpose
#define PIN_D3   0  // GPIO0        Low on boot means enter FLASH mode
#define PIN_D4   2  // GPIO2        TXD1 (must be high on boot to go to UART0 FLASH mode)
#define PIN_D5  14  // GPIO14       HSCLK
#define PIN_D6  12  // GPIO12       HMISO
#define PIN_D7  13  // GPIO13       HMOSI  RXD2
#define PIN_D8  15  // GPIO15       HCS    TXD0 (must be low on boot to enter UART0 FLASH mode)
#define PIN_D9   3  //              RXD0
#define PIN_D10  1  //              TXD0

#define PIN_MOSI 8  // SD1          FLASH and overlap mode
#define PIN_MISO 7  // SD0
#define PIN_SCLK 6  // CLK
#define PIN_HWCS 0  // D3

#define PIN_D11  9  // SD2
#define PIN_D12 10  // SD4
