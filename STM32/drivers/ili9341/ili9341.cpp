/***************************************************
  This is our library for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

 /* Heavily modified for STM32 use by A. Cadot */

#include "ili9341.h"

#include <climits>
#include <cstdlib>

#include "system.h"


ILI9341::ILI9341(SPI& spi, Pin cs, Pin dc, Pin rst):
    ILI9xxx(ILI9341::WIDTH, ILI9341::HEIGHT, spi, cs, dc, rst)
{}

bool ILI9341::init(Orientation orientation)
{
    ILI9xxx::init(orientation);

    startWrite();

    writeCommand(0xEF);
    spiWrite(0x03);
    spiWrite(0x80);
    spiWrite(0x02);

    writeCommand(0xCF);
    spiWrite(0x00);
    spiWrite(0XC1);
    spiWrite(0X30);

    writeCommand(0xED);
    spiWrite(0x64);
    spiWrite(0x03);
    spiWrite(0X12);
    spiWrite(0X81);

    writeCommand(0xE8);
    spiWrite(0x85);
    spiWrite(0x00);
    spiWrite(0x78);

    writeCommand(0xCB);
    spiWrite(0x39);
    spiWrite(0x2C);
    spiWrite(0x00);
    spiWrite(0x34);
    spiWrite(0x02);

    writeCommand(0xF7);
    spiWrite(0x20);

    writeCommand(0xEA);
    spiWrite(0x00);
    spiWrite(0x00);

    writeCommand(ILI9341::PWCTR1);    //Power control
    spiWrite(0x23);   //VRH[5:0]

    writeCommand(ILI9341::PWCTR2);    //Power control
    spiWrite(0x10);   //SAP[2:0];BT[3:0]

    writeCommand(ILI9341::VMCTR1);    //VCM control
    spiWrite(0x3e);
    spiWrite(0x28);

    writeCommand(ILI9341::VMCTR2);    //VCM control2
    spiWrite(0x86);  //--

    writeCommand(ILI9341::MADCTL);    // Memory Access Control
    spiWrite(0x48);

    writeCommand(ILI9341::VSCRSADD); // Vertical scroll
    spiWrite16(0);                 // Zero

    writeCommand(ILI9341::PIXFMT);
    spiWrite(0x55);


    writeCommand(ILI9341::FRMCTR1);
    spiWrite(0x00);
    spiWrite(0x18);

    writeCommand(ILI9341::DFUNCTR);    // Display Function Control
    spiWrite(0x08);
    spiWrite(0x82);
    spiWrite(0x27);

    writeCommand(0xF2);    // 3Gamma Function Disable
    spiWrite(0x00);

    writeCommand(ILI9341::GAMMASET);    //Gamma curve selected
    spiWrite(0x01);

    writeCommand(ILI9341::GMCTRP1);    //Set Gamma
    spiWrite(0x0F);
    spiWrite(0x31);
    spiWrite(0x2B);
    spiWrite(0x0C);
    spiWrite(0x0E);
    spiWrite(0x08);
    spiWrite(0x4E);
    spiWrite(0xF1);
    spiWrite(0x37);
    spiWrite(0x07);
    spiWrite(0x10);
    spiWrite(0x03);
    spiWrite(0x0E);
    spiWrite(0x09);
    spiWrite(0x00);

    writeCommand(ILI9341::GMCTRN1);    //Set Gamma
    spiWrite(0x00);
    spiWrite(0x0E);
    spiWrite(0x14);
    spiWrite(0x03);
    spiWrite(0x11);
    spiWrite(0x07);
    spiWrite(0x31);
    spiWrite(0xC1);
    spiWrite(0x48);
    spiWrite(0x08);
    spiWrite(0x0F);
    spiWrite(0x0C);
    spiWrite(0x31);
    spiWrite(0x36);
    spiWrite(0x0F);

    writeCommand(ILI9341::SLPOUT);    //Exit Sleep
    System::delay(120);
    writeCommand(ILI9341::DISPON);    //Display on
    System::delay(120);
    endWrite();

    setOrientation(orientation);

    return false;
}

void ILI9341::setOrientation(Orientation orientation)
{
    ILI9xxx::setOrientation(orientation);
    
    static constexpr uint8_t MADCTL_MY     = 0x80;
    static constexpr uint8_t MADCTL_MX     = 0x40;
    static constexpr uint8_t MADCTL_MV     = 0x20;
    static constexpr uint8_t MADCTL_ML     = 0x10;
    static constexpr uint8_t MADCTL_RGB    = 0x00;
    static constexpr uint8_t MADCTL_BGR    = 0x08;
    static constexpr uint8_t MADCTL_MH     = 0x04;

    uint8_t m;
    switch (orientation)
    {
        case Orientation::PORTRAIT_1:
            m = (MADCTL_MX | MADCTL_BGR);
            break;
        case Orientation::LANDSCAPE_1:
            m = (MADCTL_MV | MADCTL_BGR);
            break;
        case Orientation::PORTRAIT_2:
            m = (MADCTL_MY | MADCTL_BGR);
            break;
        default: /* Orientation::LANDSCAPE_2 */
            m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
            break;
    }

    startWrite();
    writeCommand(ILI9341::MADCTL);
    spiWrite(m);
    endWrite();
}

void ILI9341::setRotation(uint8_t m)
{
    Orientation o = Orientation::LANDSCAPE_1;
    switch (m)
    {
        case 0:
            o = Orientation::PORTRAIT_1;
            break;
        case 2:
            o = Orientation::PORTRAIT_2;
            break;
        case 3:
            o = Orientation::LANDSCAPE_2;
            break;
        default:
            break;
    }

    setOrientation(o);
}

void ILI9341::invertDisplay(bool i)
{
    startWrite();
    writeCommand(i ? ILI9341::INVON : ILI9341::INVOFF);
    endWrite();
}

void ILI9341::scrollTo(uint16_t y)
{
    startWrite();
    writeCommand(ILI9341::VSCRSADD);
    spiWrite16(y);
    endWrite();
}

uint8_t ILI9341::spiRead()
{
    uint8_t c;
    if(m_spi.read(&c, 1))
        asm("bkpt 250");
    return c;
}

void ILI9341::spiWrite(uint8_t b)
{
    if(m_spi.write(&b, 1))
        asm("bkpt 250");
}

void ILI9341::spiWrite16(uint16_t b)
{
    if(m_spi.write(reinterpret_cast<uint8_t*>(&b), 2))
        asm("bkpt 250");
}

void ILI9341::writeCommand(uint8_t cmd)
{
    m_dc.setLow();
    m_spi.write(cmd);
    m_dc.setHigh();
}

void ILI9341::writeCommand(uint8_t cmd, uint32_t data)
{
    writeCommand(cmd);

    m_spi.write((data >> 24)&0xFF);
    m_spi.write((data >> 16)&0xFF);
    m_spi.write((data >>  8)&0xFF);
    m_spi.write((data      )&0xFF);
}

void ILI9341::setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint32_t xa = ((uint32_t)x << 16) | (x+w-1);
    uint32_t ya = ((uint32_t)y << 16) | (y+h-1);

    writeCommand(ILI9341::CASET, xa); // Column addr set
    writeCommand(ILI9341::PASET, ya); // Row addr set
    writeCommand(ILI9341::RAMWR); // write to RAM
}

void ILI9341::pushColor(uint16_t color)
{
    startWrite();
    spiWrite16(color);
    endWrite();
}

uint8_t ILI9341::readcommand8(uint8_t c, uint8_t index)
{
    startWrite();
    writeCommand(0xD9);  // woo sekret command?
    spiWrite(0x10 + index);
    writeCommand(c);
    uint8_t r = spiRead();
    endWrite();

    return r;
}
