/******************************************************************
  This is our library for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required
  to interface (RST is optional)
  Adafruit invests time and resources providing this open source
  code, please support Adafruit and open-source hardware by
  purchasing products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 *******************************************************************/

 /* Extensively modified by A. Cadot (arnaud.cadot@telecom-bretagne.eu) */

#ifndef GUARD_ILI9341H
#define GUARD_ILI9341H

#include "../ili9xxx/ili9xxx.h"

class ILI9341: public ILI9xxx
{
    public:

        static constexpr uint32_t SPI_MAX_FREQUENCY = 36000000;
        static constexpr uint32_t WIDTH     = 240;
        static constexpr uint32_t HEIGHT    = 320;

    public:
        ILI9341    (SPI& _spi, Pin _CS, Pin _DC, Pin _RST = Pin());
        virtual ~ILI9341() {}

        virtual bool init(Orientation orientation = Orientation::LANDSCAPE_2);

        virtual void setOrientation (Orientation orientation);
        void invertDisplay  (bool i);
        void scrollTo       (uint16_t y);

                
    private:

        void writeCommand(uint8_t cmd);
        void writeCommand(uint8_t cmd, uint32_t data);

        void    spiWrite(uint8_t v);
        void    spiWrite16(uint16_t b);
        uint8_t spiRead(void);
        void    setRotation    (uint8_t m);
        uint8_t readcommand8(uint8_t reg, uint8_t index = 0);

        void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

	    void pushColor(uint16_t color);


        enum Commands
        {
            NOP       = 0x00,
            SWRESET   = 0x01,
            RDDID     = 0x04,
            RDDST     = 0x09,

            SLPIN     = 0x10,
            SLPOUT    = 0x11,
            PTLON     = 0x12,
            NORON     = 0x13,

            RDMODE    = 0x0A,
            RDMADCTL  = 0x0B,
            RDPIXFMT  = 0x0C,
            RDIMGFMT  = 0x0D,
            RDSELFDIAG= 0x0F,

            INVOFF    = 0x20,
            INVON     = 0x21,
            GAMMASET  = 0x26,
            ISPOFF    = 0x28,
            DISPON    = 0x29,

            CASET     = 0x2A,
            PASET     = 0x2B,
            RAMWR     = 0x2C,
            RAMRD     = 0x2E,

            PTLAR     = 0x30,
            MADCTL    = 0x36,
            VSCRSADD  = 0x37,
            PIXFMT    = 0x3A,

            FRMCTR1   = 0xB1,
            FRMCTR2   = 0xB2,
            FRMCTR3   = 0xB3,
            INVCTR    = 0xB4,
            DFUNCTR   = 0xB6,

            PWCTR1    = 0xC0,
            PWCTR2    = 0xC1,
            PWCTR3    = 0xC2,
            PWCTR4    = 0xC3,
            PWCTR5    = 0xC4,
            VMCTR1    = 0xC5,
            VMCTR2    = 0xC7,

            RDID1     = 0xDA,
            RDID2     = 0xDB,
            RDID3     = 0xDC,
            RDID4     = 0xDD,

            GMCTRP1   = 0xE0,
            GMCTRN1   = 0xE1,
        };

};

#endif
