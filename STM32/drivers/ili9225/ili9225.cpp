#include "ili9225.h"

#include "color.h"
#include "system.h"

#include <algorithm>

ILI9225::ILI9225(SPI& spi, Pin cs, Pin dc, Pin rst):
    ILI9xxx(ILI9225::HEIGHT, ILI9225::WIDTH, spi, cs, dc, rst)
{
}

bool ILI9225::init(Orientation orientation)
{
    ILI9xxx::init(orientation);

    startWrite();
    
    writeCommand(OUTPUT_CTRL,           0x011C);

	writeCommand(LCD_AC_CTRL,           0x0100);
	writeCommand(BLANK_PERIOD,          0x0808);

	writeCommand(INTERFACE_CTRL,        0x0000);
	writeCommand(OSC_CTRL,              0x0E01);
	writeCommand(RAM_ADR_1,             0x0000);
	writeCommand(RAM_ADR_2,             0x0000);

	/* Power ON sequence */
    System::delay(50);
	writeCommand(PWR_CTRL_1,            0x0900);
	writeCommand(PWR_CTRL_2,            0x1038);
    System::delay(50);
	writeCommand(PWR_CTRL_3,            0x1121);
	writeCommand(PWR_CTRL_4,            0x0065);
	writeCommand(PWR_CTRL_5,            0x5058);

	/* Set GRAM area */
	writeCommand(GATE_SCAN,             0x0000);
	writeCommand(VERTICAL_SCROLL_1,     0x00DB);
	writeCommand(VERTICAL_SCROLL_2,     0x0000);
	writeCommand(VERTICAL_SCROLL_3,     0x0000);
	writeCommand(PARTIAL_DRIVING_POS_1, 0x00DB);
	writeCommand(PARTIAL_DRIVING_POS_2, 0x0000);
	writeCommand(HORIZONTAL_WIN_ADR_1,  0x00AF);
	writeCommand(HORIZONTAL_WIN_ADR_2,  0x0000);
	writeCommand(VERTICAL_WIN_ADR_1,    0x00DB);
	writeCommand(VERTICAL_WIN_ADR_2,    0x0000);

	/* Adjust the gamma curve */
	writeCommand(GAMMA_CTRL_1,          0x0400);
	writeCommand(GAMMA_CTRL_2,          0x060B);
	writeCommand(GAMMA_CTRL_3,          0x0C0A);
	writeCommand(GAMMA_CTRL_4,          0x0105);
	writeCommand(GAMMA_CTRL_5,          0x0A0C);
	writeCommand(GAMMA_CTRL_6,          0x0B06);
	writeCommand(GAMMA_CTRL_7,          0x0004);
	writeCommand(GAMMA_CTRL_8,          0x0501);
	writeCommand(GAMMA_CTRL_9,          0x0E00);
	writeCommand(GAMMA_CTRL_10,         0x000E);
    System::delay(50);

    writeCommand(DISPLAY_CTRL,          0x1017);

    endWrite();

    setOrientation(orientation);
    
    return false;
}

void ILI9225::writeCommand(uint8_t reg)
{
    m_dc.setLow();
    m_spi.write(reg);
    m_dc.setHigh();
}

void ILI9225::writeCommand(uint8_t reg, uint16_t val)
{
    writeCommand(reg);

    m_spi.write(val>>8);
    m_spi.write(val&0xFF);
}

void ILI9225::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    setAddrWindow(x, y, w, h);

    uint16_t len = w*h;
    uint16_t temp[SPI_MAX_PIXELS_AT_ONCE];
    size_t blen = (len > SPI_MAX_PIXELS_AT_ONCE)?SPI_MAX_PIXELS_AT_ONCE:len;
    uint16_t tlen = 0;

    for (uint32_t t=0; t<blen; ++t)
        temp[t] = color;

    while(len)
    {
        tlen = (len>blen)?blen:len;
        writePixels(temp, tlen);
        len -= tlen;
    }
}

void ILI9225::fillRect(int16_t x, int16_t y,
                       int16_t w, int16_t h,
                       uint16_t color)
{
    startWrite();
    writeFillRect(x, y, w, h, color);
    endWrite();
}

void ILI9225::setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    Point2D p1 = orient(Point2D(x, y));
    Point2D p2 = orient(Point2D(x+w-1, y+h-1));

    if(p1.x() > p2.x())
        std::swap(p1.x(), p2.x());

    if(p1.y() > p2.y())
        std::swap(p1.y(), p2.y());

    writeCommand(HORIZONTAL_WIN_ADR_1,  p2.x());
    writeCommand(HORIZONTAL_WIN_ADR_2,  p1.x());
    writeCommand(VERTICAL_WIN_ADR_1,    p2.y());
    writeCommand(VERTICAL_WIN_ADR_2,    p1.y());

    /* Depending on the horizontal (resp. vertical) scan direction, 
    the memory start address (AC) will be different, because it can:
    - Increase OR decrease on the horizontal scan
    - Increase OR decrease on the veritcal scan
    Those scan settings are set by I/D0 and I/D1 on the register ENTRY_MODE.
    (see setOrientation)
    */
    writeCommand(RAM_ADR_1, (m_entryMode&EM_ID0)?p1.x():p2.x());
    writeCommand(RAM_ADR_2, (m_entryMode&EM_ID1)?p1.y():p2.y());

    writeCommand(GRAM_WRITE);
}


void ILI9225::setOrientation(Orientation orientation)
{
    ILI9xxx::setOrientation(orientation);
    
    /* Entry mode config */
    m_entryMode = EM_BGR; /* Swap blue and red channels (endianess issues) */
    switch (orientation)
    {
        case Orientation::LANDSCAPE_1:
            m_entryMode |= (EM_AM | EM_ID0);
            break;
        case Orientation::LANDSCAPE_2:
            m_entryMode |= (EM_AM | EM_ID1);
            break;
        case Orientation::PORTRAIT_1:
            m_entryMode |= (EM_ID0 | EM_ID1);
            break;
        default: /* PORTRAIT_2 */
            m_entryMode |= (0x00);
            break;
    }

    startWrite();
    writeCommand(ILI9225::ENTRY_MODE, m_entryMode);
    endWrite();
}
