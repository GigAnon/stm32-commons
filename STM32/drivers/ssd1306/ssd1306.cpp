// some of this code was written by <cstone@pobox.com> originally; 
// it is in the public domain.

#include <cstdlib>
#include <cstring>

#include "ssd1306.h"

#include <algorithm>

extern const uint8_t ssd1306_font[];

Segments::Value Segments::intToSegments(uint8_t num)
{
    switch(num)
    {
        case 0: return Segments::ZERO;
        case 1: return Segments::ONE;
        case 2: return Segments::TWO;
        case 3: return Segments::THREE;
        case 4: return Segments::FOUR;
        case 5: return Segments::FIVE;
        case 6: return Segments::SIX;
        case 7: return Segments::SEVEN;
        case 8: return Segments::EIGTH;
        case 9: return Segments::NINE;
    }
    return Segments::NONE;
}

// the memory buffer for the LCD
static uint8_t buffer[SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8] = {0};

SSD1306::SSD1306(SPI& spi, Pin dc, Pin rst, Pin cs): m_spi(&spi), m_dcPin(dc), m_rstPin(rst), m_csPin(cs)
{}

SSD1306::SSD1306(I2C& i2c, Pin rst): m_i2c(&i2c), m_rstPin(rst)
{}

void SSD1306::drawBitmap(uint8_t x, uint8_t y, 
			            const uint8_t *bitmap, uint8_t w, uint8_t h,
			            uint8_t color)
{
    for (uint8_t j=0; j<h; j++)
        for (uint8_t i=0; i<w; i++ )
            if (bitmap[i + (j/8)*w] & (1 << (j%8)))
                setPixel(x+i, y+j, color);
}

void SSD1306::drawString(uint8_t x, uint8_t line, const char *c)
{
  while (c[0] != 0)
  {
    drawChar(x, line, c[0]);
    ++c;
    x += 6; // 6 pixels wide
    if (x + 6 >= SSD1306_LCDWIDTH)
    {
        x = 0;    // ran out of this line
        ++line;
    }
    if (line >= (SSD1306_LCDHEIGHT/8))
        return;        // ran out of space :(
  }
}

inline uint8_t reverse(uint8_t b)
{
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void  SSD1306::drawChar(uint8_t x, uint8_t line, uint8_t c)
{
    if ((line >= SSD1306_LCDHEIGHT/8) || (x >= (SSD1306_LCDWIDTH - 6)))
        return;

    if(c > 0x7E)
        return;

    if(m_displayInverted)
    {
        uint32_t t = (SSD1306_LCDHEIGHT/8 - line - 1)*128 + SSD1306_LCDWIDTH - 7 - x - 1 + 4;

        for (uint8_t i=0; i<5; ++i)
            buffer[t-i] = reverse(ssd1306_font[c*5+i]);
    }
    else
    {
        uint32_t t = x+line*128;
        for (uint8_t i=0; i<5; ++i)
            buffer[t+i] = ssd1306_font[c*5+i];
    }
}

void SSD1306::drawSevenSegments(uint8_t x, uint8_t y, uint8_t num, uint8_t size)
{
    drawSevenSegments(x, y, Segments::intToSegments(num), size);
}

void SSD1306::drawSevenSegments(uint8_t x, uint8_t y, Segments::Value seg, uint8_t size)
{
    uint8_t segWidth;
    uint8_t segHeigth   = size/2 - 2;

    if(size > 20)
        segWidth = 3;
    else if(size > 15)
        segWidth = 2;
    else
        segWidth = 1;

    if(seg&Segments::TOP_LEFT)
        fillRect(x, y+1, segWidth, segHeigth);
    if(seg&Segments::TOP_RIGHT)
        fillRect(x+segHeigth+segWidth+2, y+1, segWidth, segHeigth);
    if(seg&Segments::BOTTOM_LEFT)
        fillRect(x, y + segHeigth + 2, segWidth, segHeigth);
    if(seg&Segments::BOTTOM_RIGHT)
        fillRect(x+segHeigth+segWidth+2, y + segHeigth + 2, segWidth, segHeigth);
    if(seg&Segments::TOP)
        fillRect(x+segWidth+1, y+1, segHeigth,  segWidth);
    if(seg&Segments::MIDDLE)
        fillRect(x+segWidth+1, y+segHeigth, segHeigth, segWidth);
    if(seg&Segments::BOTTOM)
        fillRect(x+segWidth+1, y+size-segWidth-2, segHeigth, segWidth);
}

void SSD1306::drawMultipleSegments(uint8_t x, uint8_t y, uint16_t num, uint8_t size)
{
    if( x >= SSD1306_LCDWIDTH ||
        y >= SSD1306_LCDHEIGHT)
        return;

    uint16_t tens = 1;

    /* fast log10, basically */
    for(uint16_t tmp=num; tmp > 10; tens *= 10, tmp /= 10);

    for(;tens;tens /= 10)
    {
        uint8_t digits = num / tens;
        drawSevenSegments(x, y, num / tens, size);

        x += size;
        num -= digits*tens;

        if(x >= SSD1306_LCDWIDTH)
            break;
    }
}

// bresenham's algorithm - thx wikpedia
void SSD1306::drawLine( uint8_t x0, uint8_t y0,
                        uint8_t x1, uint8_t y1, 
		                uint8_t color)
{
    uint8_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep)
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    uint8_t dx = x1 - x0;
    uint8_t dy = abs(y1 - y0);

    int8_t err = dx / 2;
    int8_t ystep = (y0<y1)?1:-1;


    for (; x0<x1; ++x0) 
    {
        if (steep)
            setPixel(y0, x0, color);
        else
            setPixel(x0, y0, color);

        err -= dy;
        if (err < 0)
        {
            y0 += ystep;
            err += dx;
        }
    }
}

// filled rectangle
void SSD1306::fillRect( uint8_t x, uint8_t y,
                        uint8_t w, uint8_t h, 
		                uint8_t color)
{
    for (uint8_t i=x; i<x+w; i++)
        for (uint8_t j=y; j<y+h; j++)
            setPixel(i, j, color);
}

// draw a rectangle
void SSD1306::drawRect( uint8_t x, uint8_t y,
                        uint8_t w, uint8_t h, 
		                uint8_t color)
{
    for (uint8_t i=x; i<x+w; ++i)
    {
        setPixel(i, y, color);
        setPixel(i, y+h-1, color);
    }
    for (uint8_t i=y; i<y+h; ++i)
    {
        setPixel(x, i, color);
        setPixel(x+w-1, i, color);
    }
}

// draw a circle outline
void SSD1306::drawCircle(   uint8_t x0, uint8_t y0, uint8_t r, 
			                uint8_t color)
{
    int8_t f = 1 - r;
    int8_t ddF_x = 1;
    int8_t ddF_y = -2 * r;
    int8_t x = 0;
    int8_t y = r;

    setPixel(x0, y0+r, color);
    setPixel(x0, y0-r, color);
    setPixel(x0+r, y0, color);
    setPixel(x0-r, y0, color);

    while (x<y)
    {
        if (f >= 0)
        {
            --y;
            ddF_y += 2;
            f += ddF_y;
        }
        ++x;
        ddF_x += 2;
        f += ddF_x;
  
        setPixel(x0 + x, y0 + y, color);
        setPixel(x0 - x, y0 + y, color);
        setPixel(x0 + x, y0 - y, color);
        setPixel(x0 - x, y0 - y, color);
    
        setPixel(x0 + y, y0 + x, color);
        setPixel(x0 - y, y0 + x, color);
        setPixel(x0 + y, y0 - x, color);
        setPixel(x0 - y, y0 - x, color);
    }
}

void SSD1306::fillCircle(   uint8_t x0, uint8_t y0, uint8_t r, 
			                uint8_t color)
{
    int8_t f = 1 - r;
    int8_t ddF_x = 1;
    int8_t ddF_y = -2 * r;
    int8_t x = 0;
    int8_t y = r;

    for (uint8_t i=y0-r; i<=y0+r; i++)
        setPixel(x0, i, color);


    while (x<y)
    {
        if (f >= 0)
        {
            --y;
            ddF_y += 2;
            f += ddF_y;
        }
        ++x;
        ddF_x += 2;
        f += ddF_x;
  
        for (uint8_t i=y0-y; i<=y0+y; ++i)
        {
            setPixel(x0+x, i, color);
            setPixel(x0-x, i, color);
        } 
        for (uint8_t i=y0-x; i<=y0+x; ++i)
        {
            setPixel(x0+y, i, color);
            setPixel(x0-y, i, color);
        }    
    }
}

// the most basic function, set a single pixel
void SSD1306::setPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if ((x >= SSD1306_LCDWIDTH) || (y >= SSD1306_LCDHEIGHT))
        return;

    if(m_displayInverted)
    {
        y = SSD1306_LCDHEIGHT   - y - 1;
        x = SSD1306_LCDWIDTH    - x - 1;
    }

    // x is which column
    if (color == WHITE)
        buffer[x+(y/8)*SSD1306_LCDWIDTH] |= (1<<(y%8));  
    else
        buffer[x+(y/8)*SSD1306_LCDWIDTH] &= ~(1<<(y%8)); 
}

void SSD1306::init(SupplyType vccstate)
{
	// set pin directions
	
	m_dcPin. init(GPIO_MODE_OUTPUT_PP);
	m_rstPin.init(GPIO_MODE_OUTPUT_PP);
	m_csPin. init(GPIO_MODE_OUTPUT_PP);
	
    if(m_rstPin.isValid())
    {
	    m_rstPin.setHigh();
	    HAL_Delay(1);
	    m_rstPin.setLow();
	    HAL_Delay(1);
	    m_rstPin.setHigh();
    }


    // Init sequence
    command(DISPLAYOFF);                    // 0xAE
    command(SETDISPLAYCLOCKDIV);            // 0xD5
    command(0x80);                          // the suggested ratio 0x80

    command(SETMULTIPLEX);                  // 0xA8
    command(SSD1306_LCDHEIGHT - 1);

    command(SETDISPLAYOFFSET);              // 0xD3
    command(0x0);                           // no offset
    command(SETSTARTLINE | 0x0);            // line #0
    command(CHARGEPUMP);                    // 0x8D
    command((vccstate == SupplyType::External)?0x10:0x14);
    command(MEMORYMODE);                    // 0x20
    command(0x00);                          // 0x0 act like ks0108
    command(SEGREMAP | 0x1);
    command(COMSCANDEC);

    #if defined SSD1306_128_32
    command(SETCOMPINS);                    // 0xDA
    command(0x02);
    command(SETCONTRAST);                   // 0x81
    command(0x8F);

    #elif defined SSD1306_128_64
    command(SETCOMPINS);                    // 0xDA
    command(0x12);
    command(SETCONTRAST);                   // 0x81
    command(0xFF);

    #elif defined SSD1306_96_16
    command(SETCOMPINS);                    // 0xDA
    command(0x2);   //ada x12
    command(SETCONTRAST);                   // 0x81
    command((vccstate == SupplyType::External)?0x10:0xAF);

    #endif

    command(SETPRECHARGE);                  // 0xd9
    command((vccstate == SupplyType::External)?0x22:0xF1);
    command(SETVCOMDETECT);                 // 0xDB
    command(0x40);
    command(DISPLAYALLON_RESUME);           // 0xA4
    command(NORMALDISPLAY);                 // 0xA6

    command(DEACTIVATE_SCROLL);

    command(DISPLAYON);//--turn on oled panel

    display();
}

void SSD1306::setInverted(bool inverted)
{
    m_displayInverted = inverted;
}

void SSD1306::setColorInverted(bool inverted)
{
    command(inverted?INVERTDISPLAY:NORMALDISPLAY);
}

void SSD1306::command(uint8_t c)
{ 
    if(m_spi)
    {
        m_csPin.setHigh();
        m_dcPin.setLow();
        m_csPin.setLow();
    
	    m_spi->write(c);
    
	    m_csPin.setHigh();
    }
    else
    {
        ByteArray d(2, 0x00);
        d[0] = 0x00;
        d[1] = c;
        m_i2c->write(SSD1306::I2C_ADR, d);
    }
}

void SSD1306::setContrast(uint8_t val)
{
    command(SETCONTRAST);
    command(val);
}


void SSD1306::display()
{
    /*command(SETLOWCOLUMN    | 0x0);  // low col = 0
    command(SETHIGHCOLUMN   | 0x0);  // hi col = 0
    command(SETSTARTLINE    | 0x0); // line #0*/

    command(COLUMNADDR);
    command(0);
    command(SSD1306_LCDWIDTH-1);

    command(PAGEADDR);
    command(0);
    #if SSD1306_LCDHEIGHT == 64
    command(7); // Page end address
    #endif
    #if SSD1306_LCDHEIGHT == 32
    command(3); // Page end address
    #endif
    #if SSD1306_LCDHEIGHT == 16
    command(1); // Page end address
    #endif
    
    if(m_spi)
    {
        m_csPin.setHigh();
        m_dcPin.setHigh();
        m_csPin.setLow();

        m_spi->write(buffer, SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8);

        m_csPin.setHigh();
    }
    else
    {
        constexpr uint16_t packetSize = 128;
        ByteArray b;

        b.reserve(packetSize+1);

        for (uint16_t i=0; i<(SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8); i+=packetSize)
        {
            b.clear();
            b.append(0x40);

            for(uint16_t j=0;j<packetSize;++j)
                b.append(buffer[i+j]);
            
            m_i2c->write(SSD1306::I2C_ADR, b);
        }
    }
}

// clear everything
void SSD1306::clear()
{
    memset(buffer, 0, (SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8));
}

void SSD1306::clearDisplay()
{
    clear();
    display();
}
