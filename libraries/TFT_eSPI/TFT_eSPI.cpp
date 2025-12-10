/***************************************************
  Arduino TFT graphics library targeted at 32-bit
  processors such as ESP32, ESP8266 and STM32.

  This is a stand-alone library that contains the
  hardware driver, the graphics functions and the
  proportional fonts.

  The larger fonts are Run Length Encoded to reduce their
  size.

  Created by Bodmer 2/12/16
  Last update by Bodmer 20/03/20
 ****************************************************/

#include "TFT_eSPI.h"

#if defined (ESP32)
  #if defined(CONFIG_IDF_TARGET_ESP32S3)
    #include "Processors/TFT_eSPI_ESP32_S3.c" // Tested with SPI and 8-bit parallel
  #elif defined(CONFIG_IDF_TARGET_ESP32C3)
    #include "Processors/TFT_eSPI_ESP32_C3.c" // Tested with SPI (8-bit parallel will probably work too!)
  #else
    #include "Processors/TFT_eSPI_ESP32.c"
  #endif
#elif defined (ARDUINO_ARCH_ESP8266)
  #include "Processors/TFT_eSPI_ESP8266.c"
#elif defined (STM32) // (_VARIANT_ARDUINO_STM32_) stm32_def.h
  #include "Processors/TFT_eSPI_STM32.c"
#elif defined (ARDUINO_ARCH_RP2040)  || defined (ARDUINO_ARCH_MBED) // Raspberry Pi Pico
  #include "Processors/TFT_eSPI_RP2040.c"
#else
  #include "Processors/TFT_eSPI_Generic.c"
#endif

#ifndef SPI_BUSY_CHECK
  #define SPI_BUSY_CHECK
#endif

// Clipping macro for pushImage
#define PI_CLIP                                        \
  if (_vpOoB) return;                                  \
  x+= _xDatum;                                         \
  y+= _yDatum;                                         \
                                                       \
  if ((x >= _vpW) || (y >= _vpH)) return;              \
                                                       \
  int32_t dx = 0;                                      \
  int32_t dy = 0;                                      \
  int32_t dw = w;                                      \
  int32_t dh = h;                                      \
                                                       \
  if (x < _vpX) { dx = _vpX - x; dw -= dx; x = _vpX; } \
  if (y < _vpY) { dy = _vpY - y; dh -= dy; y = _vpY; } \
                                                       \
  if ((x + dw) > _vpW ) dw = _vpW - x;                 \
  if ((y + dh) > _vpH ) dh = _vpH - y;                 \
                                                       \
  if (dw < 1 || dh < 1) return;

/***************************************************************************************
** Function name:           Legacy - deprecated
** Description:             Start/end transaction
***************************************************************************************/
  void TFT_eSPI::spi_begin()       {begin_tft_write();}
  void TFT_eSPI::spi_end()         {  end_tft_write();}
  void TFT_eSPI::spi_begin_read()  {begin_tft_read(); }
  void TFT_eSPI::spi_end_read()    {  end_tft_read(); }

/***************************************************************************************
** Function name:           begin_tft_write (was called spi_begin)
** Description:             Start SPI transaction for writes and select TFT
***************************************************************************************/
inline void TFT_eSPI::begin_tft_write(void){
  if (locked) {
    locked = false; // Flag to show SPI access now unlocked
#if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS) && !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
    spi.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, TFT_SPI_MODE));
#endif
    CS_L;
    SET_BUS_WRITE_MODE;  // Some processors (e.g. ESP32) allow recycling the tx buffer when rx is not used
  }
}

// Non-inlined version to permit override
void TFT_eSPI::begin_nin_write(void){
  if (locked) {
    locked = false; // Flag to show SPI access now unlocked
#if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS) && !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
    spi.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, TFT_SPI_MODE));
#endif
    CS_L;
    SET_BUS_WRITE_MODE;  // Some processors (e.g. ESP32) allow recycling the tx buffer when rx is not used
  }
}

/***************************************************************************************
** Function name:           end_tft_write (was called spi_end)
** Description:             End transaction for write and deselect TFT
***************************************************************************************/
inline void TFT_eSPI::end_tft_write(void){
  if(!inTransaction) {      // Flag to stop ending transaction during multiple graphics calls
    if (!locked) {          // Locked when beginTransaction has been called
      locked = true;        // Flag to show SPI access now locked
      SPI_BUSY_CHECK;       // Check send complete and clean out unused rx data
      CS_H;
      SET_BUS_READ_MODE;    // In case bus has been configured for tx only
#if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS) && !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
      spi.endTransaction();
#endif
    }
  }
}

// Non-inlined version to permit override
inline void TFT_eSPI::end_nin_write(void){
  if(!inTransaction) {      // Flag to stop ending transaction during multiple graphics calls
    if (!locked) {          // Locked when beginTransaction has been called
      locked = true;        // Flag to show SPI access now locked
      SPI_BUSY_CHECK;       // Check send complete and clean out unused rx data
      CS_H;
      SET_BUS_READ_MODE;    // In case SPI has been configured for tx only
#if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS) && !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
      spi.endTransaction();
#endif
    }
  }
}

/***************************************************************************************
** Function name:           begin_tft_read  (was called spi_begin_read)
** Description:             Start transaction for reads and select TFT
***************************************************************************************/
// Reads require a lower SPI clock rate than writes
inline void TFT_eSPI::begin_tft_read(void){
  DMA_BUSY_CHECK; // Wait for any DMA transfer to complete before changing SPI settings
#if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS) && !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
  if (locked) {
    locked = false;
    spi.beginTransaction(SPISettings(SPI_READ_FREQUENCY, MSBFIRST, TFT_SPI_MODE));
    CS_L;
  }
#else
  #if !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
    spi.setFrequency(SPI_READ_FREQUENCY);
  #endif
   CS_L;
#endif
  SET_BUS_READ_MODE;
}

/***************************************************************************************
** Function name:           end_tft_read (was called spi_end_read)
** Description:             End transaction for reads and deselect TFT
***************************************************************************************/
inline void TFT_eSPI::end_tft_read(void){
#if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS) && !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
  if(!inTransaction) {
    if (!locked) {
      locked = true;
      CS_H;
      spi.endTransaction();
    }
  }
#else
  #if !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
    spi.setFrequency(SPI_FREQUENCY);
  #endif
   if(!inTransaction) {CS_H;}
#endif
  SET_BUS_WRITE_MODE;
}


/***************************************************************************************
** Function name:           TFT_eSPI
** Description:             Constructor , we must use hardware SPI pins
***************************************************************************************/
TFT_eSPI::TFT_eSPI(int16_t w, int16_t h)
{
  _init_width  = _width  = w; // Set by specific xxxxx_Defines.h file or by users sketch
  _init_height = _height = h; // Set by specific xxxxx_Defines.h file or by users sketch

  // Reset the viewport to the whole screen


  rotation  = 0;



 

  _swapBytes = true;   // Do not swap colour bytes by default

  locked = true;           // Transaction mutex lock flag to ensure begin/endTranaction pairing
  inTransaction = false;   // Flag to prevent multiple sequential functions to keep bus access open
  lockTransaction = false; // start/endWrite lock flag to allow sketch to keep SPI bus access open

  _booted   = true;     // Default attributes
  


#if defined (ESP32) && defined (CONFIG_SPIRAM_SUPPORT)
  if (psramFound()) _psram_enable = true; // Enable the use of PSRAM (if available)
  else
#endif
  _psram_enable = false;

 


// Legacy support for bit GPIO masks
  cspinmask = 0;
  dcpinmask = 0;
  wrpinmask = 0;
  sclkpinmask = 0;


}
/***************************************************************************************
** Function name:           initBus
** Description:             initialise the SPI or parallel bus
***************************************************************************************/
void TFT_eSPI::initBus(void) {

#ifdef TFT_CS
  if (TFT_CS >= 0) {
    pinMode(TFT_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH); // Chip select high (inactive)
  }
#endif

// Configure chip select for touchscreen controller if present
#ifdef TOUCH_CS
  if (TOUCH_CS >= 0) {
    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH); // Chip select high (inactive)
  }
#endif

// In parallel mode and with the RP2040 processor, the TFT_WR line is handled in the  PIO
#if defined (TFT_WR) && !defined (ARDUINO_ARCH_RP2040) && !defined (ARDUINO_ARCH_MBED)
  if (TFT_WR >= 0) {
    pinMode(TFT_WR, OUTPUT);
    digitalWrite(TFT_WR, HIGH); // Set write strobe high (inactive)
  }
#endif

#ifdef TFT_DC
  if (TFT_DC >= 0) {
    pinMode(TFT_DC, OUTPUT);
    digitalWrite(TFT_DC, HIGH); // Data/Command high = data mode
  }
#endif

#ifdef TFT_RST
  if (TFT_RST >= 0) {
    pinMode(TFT_RST, OUTPUT);
    digitalWrite(TFT_RST, HIGH); // Set high, do not share pin with another SPI device
  }
#endif

#if defined (TFT_PARALLEL_8_BIT)

  // Make sure read is high before we set the bus to output
  if (TFT_RD >= 0) {
    pinMode(TFT_RD, OUTPUT);
    digitalWrite(TFT_RD, HIGH);
  }

  #if  !defined (ARDUINO_ARCH_RP2040)  && !defined (ARDUINO_ARCH_MBED)// PIO manages pins
    // Set TFT data bus lines to output
    pinMode(TFT_D0, OUTPUT); digitalWrite(TFT_D0, HIGH);
    pinMode(TFT_D1, OUTPUT); digitalWrite(TFT_D1, HIGH);
    pinMode(TFT_D2, OUTPUT); digitalWrite(TFT_D2, HIGH);
    pinMode(TFT_D3, OUTPUT); digitalWrite(TFT_D3, HIGH);
    pinMode(TFT_D4, OUTPUT); digitalWrite(TFT_D4, HIGH);
    pinMode(TFT_D5, OUTPUT); digitalWrite(TFT_D5, HIGH);
    pinMode(TFT_D6, OUTPUT); digitalWrite(TFT_D6, HIGH);
    pinMode(TFT_D7, OUTPUT); digitalWrite(TFT_D7, HIGH);
  #endif

  PARALLEL_INIT_TFT_DATA_BUS;

#endif
}

/***************************************************************************************
** Function name:           begin
** Description:             Included for backwards compatibility
***************************************************************************************/
void TFT_eSPI::begin(uint8_t tc)
{
 init(tc);
}


/***************************************************************************************
** Function name:           init (tc is tab colour for ST7735 displays only)
** Description:             Reset, then initialise the TFT display registers
***************************************************************************************/
void TFT_eSPI::init(uint8_t tc)
{
  if (_booted)
  {
    initBus();

#if !defined (ESP32) && !defined(TFT_PARALLEL_8_BIT) && !defined(ARDUINO_ARCH_RP2040) && !defined (ARDUINO_ARCH_MBED)
  // Legacy bitmasks for GPIO
  #if defined (TFT_CS) && (TFT_CS >= 0)
    cspinmask = (uint32_t) digitalPinToBitMask(TFT_CS);
  #endif

  #if defined (TFT_DC) && (TFT_DC >= 0)
    dcpinmask = (uint32_t) digitalPinToBitMask(TFT_DC);
  #endif

  #if defined (TFT_WR) && (TFT_WR >= 0)
    wrpinmask = (uint32_t) digitalPinToBitMask(TFT_WR);
  #endif

  #if defined (TFT_SCLK) && (TFT_SCLK >= 0)
    sclkpinmask = (uint32_t) digitalPinToBitMask(TFT_SCLK);
  #endif

  #if defined (TFT_SPI_OVERLAP) && defined (ARDUINO_ARCH_ESP8266)
    // Overlap mode SD0=MISO, SD1=MOSI, CLK=SCLK must use D3 as CS
    //    pins(int8_t sck, int8_t miso, int8_t mosi, int8_t ss);
    //spi.pins(        6,          7,           8,          0);
    spi.pins(6, 7, 8, 0);
  #endif

  spi.begin(); // This will set HMISO to input

#else
  #if !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
    #if defined (TFT_MOSI) && !defined (TFT_SPI_OVERLAP) && !defined(ARDUINO_ARCH_RP2040) && !defined (ARDUINO_ARCH_MBED)
      spi.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, -1); // This will set MISO to input
    #else
      spi.begin(); // This will set MISO to input
    #endif
  #endif
#endif
    lockTransaction = false;
    inTransaction = false;
    locked = true;

    INIT_TFT_DATA_BUS;


#if defined (TFT_CS) && !defined(RP2040_PIO_INTERFACE)
  // Set to output once again in case MISO is used for CS
  if (TFT_CS >= 0) {
    pinMode(TFT_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH); // Chip select high (inactive)
  }
#elif defined (ARDUINO_ARCH_ESP8266) && !defined (TFT_PARALLEL_8_BIT) && !defined (RP2040_PIO_SPI)
  spi.setHwCs(1); // Use hardware SS toggling
#endif


  // Set to output once again in case MISO is used for DC
#if defined (TFT_DC) && !defined(RP2040_PIO_INTERFACE)
  if (TFT_DC >= 0) {
    pinMode(TFT_DC, OUTPUT);
    digitalWrite(TFT_DC, HIGH); // Data/Command high = data mode
  }
#endif

    _booted = false;
    end_tft_write();
  } // end of: if just _booted

  // Toggle RST low to reset
#ifdef TFT_RST
  #if !defined(RP2040_PIO_INTERFACE)
    // Set to output once again in case MISO is used for TFT_RST
    if (TFT_RST >= 0) {
      pinMode(TFT_RST, OUTPUT);
    }
  #endif
  if (TFT_RST >= 0) {
    writecommand(0x00); // Put SPI bus in known state for TFT with CS tied low
    digitalWrite(TFT_RST, HIGH);
    delay(5);
    digitalWrite(TFT_RST, LOW);
    delay(20);
    digitalWrite(TFT_RST, HIGH);
  }
  else writecommand(TFT_SWRST); // Software reset
#else
  writecommand(TFT_SWRST); // Software reset
#endif

  delay(150); // Wait for reset to complete

  begin_tft_write();

  tc = tc; // Suppress warning

#include "TFT_Drivers/GC9A01_Init.h"



#ifdef TFT_INVERSION_ON
  writecommand(TFT_INVON);
#endif

#ifdef TFT_INVERSION_OFF
  writecommand(TFT_INVOFF);
#endif

  end_tft_write();

  setRotation(rotation);

#if defined (TFT_BL) && defined (TFT_BACKLIGHT_ON)
  if (TFT_BL >= 0) {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
  }
#else
  #if defined (TFT_BL) && defined (M5STACK)
    // Turn on the back-light LED
    if (TFT_BL >= 0) {
      pinMode(TFT_BL, OUTPUT);
      digitalWrite(TFT_BL, HIGH);
    }
  #endif
#endif
}


/***************************************************************************************
** Function name:           setRotation
** Description:             rotate the screen orientation m = 0-3 or 4-7 for BMP drawing
***************************************************************************************/
void TFT_eSPI::setRotation(uint8_t m)
{

  begin_tft_write();
  #include "TFT_Drivers/GC9A01_Rotation.h"
  delayMicroseconds(10);

  end_tft_write();


}


/***************************************************************************************
** Function name:           getRotation
** Description:             Return the rotation value (as used by setRotation())
***************************************************************************************/
uint8_t TFT_eSPI::getRotation(void)
{
  return rotation;
}







/***************************************************************************************
** Function name:           writecommand
** Description:             Send an 8-bit command to the TFT
***************************************************************************************/
#ifndef RM68120_DRIVER
void TFT_eSPI::writecommand(uint8_t c)
{
  begin_tft_write();

  DC_C;

  tft_Write_8(c);

  DC_D;

  end_tft_write();
}
#else
void TFT_eSPI::writecommand(uint16_t c)
{
  begin_tft_write();

  DC_C;

  tft_Write_16(c);

  DC_D;

  end_tft_write();

}
void TFT_eSPI::writeRegister8(uint16_t c, uint8_t d)
{
  begin_tft_write();

  DC_C;

  tft_Write_16(c);

  DC_D;

  tft_Write_8(d);

  end_tft_write();

}
void TFT_eSPI::writeRegister16(uint16_t c, uint16_t d)
{
  begin_tft_write();

  DC_C;

  tft_Write_16(c);

  DC_D;

  tft_Write_16(d);

  end_tft_write();

}

#endif

/***************************************************************************************
** Function name:           writedata
** Description:             Send a 8-bit data value to the TFT
***************************************************************************************/
void TFT_eSPI::writedata(uint8_t d)
{
  begin_tft_write();

  DC_D;        // Play safe, but should already be in data mode

  tft_Write_8(d);

  CS_L;        // Allow more hold time for low VDI rail

  end_tft_write();
}


/***************************************************************************************
** Function name:           readcommand8
** Description:             Read a 8-bit data value from an indexed command register
***************************************************************************************/
uint8_t TFT_eSPI::readcommand8(uint8_t cmd_function, uint8_t index)
{
  uint8_t reg = 0;
#if defined(TFT_PARALLEL_8_BIT) || defined(RP2040_PIO_INTERFACE)

  writecommand(cmd_function); // Sets DC and CS high

  busDir(GPIO_DIR_MASK, INPUT);

  CS_L;

  // Read nth parameter (assumes caller discards 1st parameter or points index to 2nd)
  while(index--) reg = readByte();

  busDir(GPIO_DIR_MASK, OUTPUT);

  CS_H;

#else // SPI interface
  // Tested with ILI9341 set to Interface II i.e. IM [3:0] = "1101"
  begin_tft_read();
  index = 0x10 + (index & 0x0F);

  DC_C; tft_Write_8(0xD9);
  DC_D; tft_Write_8(index);

  CS_H; // Some displays seem to need CS to be pulsed here, or is just a delay needed?
  CS_L;

  DC_C; tft_Write_8(cmd_function);
  DC_D;
  reg = tft_Read_8();

  end_tft_read();
#endif
  return reg;
}


/***************************************************************************************
** Function name:           readcommand16
** Description:             Read a 16-bit data value from an indexed command register
***************************************************************************************/
uint16_t TFT_eSPI::readcommand16(uint8_t cmd_function, uint8_t index)
{
  uint32_t reg;

  reg  = (readcommand8(cmd_function, index + 0) <<  8);
  reg |= (readcommand8(cmd_function, index + 1) <<  0);

  return reg;
}


/***************************************************************************************
** Function name:           readcommand32
** Description:             Read a 32-bit data value from an indexed command register
***************************************************************************************/
uint32_t TFT_eSPI::readcommand32(uint8_t cmd_function, uint8_t index)
{
  uint32_t reg;

  reg  = ((uint32_t)readcommand8(cmd_function, index + 0) << 24);
  reg |= ((uint32_t)readcommand8(cmd_function, index + 1) << 16);
  reg |= ((uint32_t)readcommand8(cmd_function, index + 2) <<  8);
  reg |= ((uint32_t)readcommand8(cmd_function, index + 3) <<  0);

  return reg;
}

/***************************************************************************************
** Function name:           setWindow
** Description:             define an area to receive a stream of pixels
***************************************************************************************/
// Chip select stays low, call begin_tft_write first. Use setAddrWindow() from sketches
void TFT_eSPI::setWindow(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{

    SPI_BUSY_CHECK;
    DC_C; tft_Write_8(TFT_CASET);
    DC_D; tft_Write_32C(x0, x1);
    DC_C; tft_Write_8(TFT_PASET);
    DC_D; tft_Write_32C(y0, y1);
    DC_C; tft_Write_8(TFT_RAMWR);
    DC_D;

}
/***************************************************************************************
** Function name:           setAddrWindow
** Description:             define an area to receive a stream of pixels
***************************************************************************************/
// Chip select is high at the end of this function
void TFT_eSPI::setAddrWindow(int32_t x0, int32_t y0, int32_t w, int32_t h)
{
  begin_tft_write();

  setWindow(x0, y0, x0 + w - 1, y0 + h - 1);

  end_tft_write();
}



/***************************************************************************************
** Function name:           readAddrWindow
** Description:             define an area to read a stream of pixels
***************************************************************************************/
void TFT_eSPI::readAddrWindow(int32_t xs, int32_t ys, int32_t w, int32_t h)
{
  //begin_tft_write(); // Must be called before readAddrWindow or CS set low

  int32_t xe = xs + w - 1;
  int32_t ye = ys + h - 1;
  // Column addr set
  DC_C; tft_Write_8(TFT_CASET);
  DC_D; tft_Write_32C(xs, xe);

  // Row addr set
  DC_C; tft_Write_8(TFT_PASET);
  DC_D; tft_Write_32C(ys, ye);

  // Read CGRAM command
  DC_C; tft_Write_8(TFT_RAMRD);

  DC_D;

}




/***************************************************************************************
** Function name:           startWrite
** Description:             begin transaction with CS low, MUST later call endWrite
***************************************************************************************/
void TFT_eSPI::startWrite(void)
{
  begin_tft_write();
  lockTransaction = true; // Lock transaction for all sequentially run sketch functions
  inTransaction = true;
}

/***************************************************************************************
** Function name:           endWrite
** Description:             end transaction with CS high
***************************************************************************************/
void TFT_eSPI::endWrite(void)
{
  lockTransaction = false; // Release sketch induced transaction lock
  inTransaction = false;
  DMA_BUSY_CHECK;          // Safety check - user code should have checked this!
  end_tft_write();         // Release SPI bus
}



/***************************************************************************************
** Function name:           pushColors
** Description:             push an array of pixels for 16-bit raw image drawing
***************************************************************************************/
// Assumed that setAddrWindow() has previously been called
// len is number of bytes, not pixels
void TFT_eSPI::pushColors(uint8_t *data, uint32_t len)
{
  begin_tft_write();

  pushPixels(data, len>>1);

  end_tft_write();
}


/***************************************************************************************
** Function name:           pushColors
** Description:             push an array of pixels, for image drawing
***************************************************************************************/
void TFT_eSPI::pushColors(uint16_t *data, uint32_t len)
{
  begin_tft_write();


  pushPixels(data, len);

 
  end_tft_write();
}


/***************************************************************************************
** Function name:           color565
** Description:             convert three 8-bit RGB levels to a 16-bit colour value
***************************************************************************************/
uint16_t TFT_eSPI::color565(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


/***************************************************************************************
** Function name:           color16to8
** Description:             convert 16-bit colour to an 8-bit 332 RGB colour value
***************************************************************************************/
uint8_t TFT_eSPI::color16to8(uint16_t c)
{
  return ((c & 0xE000)>>8) | ((c & 0x0700)>>6) | ((c & 0x0018)>>3);
}


/***************************************************************************************
** Function name:           color8to16
** Description:             convert 8-bit colour to a 16-bit 565 colour value
***************************************************************************************/
uint16_t TFT_eSPI::color8to16(uint8_t color)
{
  uint8_t  blue[] = {0, 11, 21, 31}; // blue 2 to 5-bit colour lookup table
  uint16_t color16 = 0;

  //        =====Green=====     ===============Red==============
  color16  = (color & 0x1C)<<6 | (color & 0xC0)<<5 | (color & 0xE0)<<8;
  //        =====Green=====    =======Blue======
  color16 |= (color & 0x1C)<<3 | blue[color & 0x03];

  return color16;
}

/***************************************************************************************
** Function name:           color16to24
** Description:             convert 16-bit colour to a 24-bit 888 colour value
***************************************************************************************/
uint32_t TFT_eSPI::color16to24(uint16_t color565)
{
  uint8_t r = (color565 >> 8) & 0xF8; r |= (r >> 5);
  uint8_t g = (color565 >> 3) & 0xFC; g |= (g >> 6);
  uint8_t b = (color565 << 3) & 0xF8; b |= (b >> 5);

  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | ((uint32_t)b << 0);
}

/***************************************************************************************
** Function name:           color24to16
** Description:             convert 24-bit colour to a 16-bit 565 colour value
***************************************************************************************/
uint32_t TFT_eSPI::color24to16(uint32_t color888)
{
  uint16_t r = (color888 >> 8) & 0xF800;
  uint16_t g = (color888 >> 5) & 0x07E0;
  uint16_t b = (color888 >> 3) & 0x001F;

  return (r | g | b);
}


