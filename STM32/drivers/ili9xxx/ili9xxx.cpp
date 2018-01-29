#include "ili9xxx.h"

#include "system.h"

#include <memory>
#include <cmath>

ILI9xxx::ILI9xxx(   uint16_t width, uint16_t height,
                    SPI& spi, Pin cs, Pin dc, Pin rst)
    : Display(width, height), m_spi(spi), m_cs(cs), m_dc(dc), m_rst(rst)
{
}

bool ILI9xxx::init(Orientation orientation)
{
    m_dc.init(GPIO_MODE_OUTPUT_PP);
    m_dc.setLow();
    m_cs.init(GPIO_MODE_OUTPUT_PP);
    m_cs.setHigh();

    // toggle RST low to reset
    if (m_rst.isValid())
    {
        m_rst.init(GPIO_MODE_OUTPUT_PP);
        reset();
    }

    setOrientation(orientation);

    return false;
}

void ILI9xxx::reset()
{
    if (m_rst.isValid())
    {
        m_rst.setHigh();
        System::delay(5);
        m_rst.setLow();
        System::delay(20);
        m_rst.setHigh();
        System::delay(100);
    }
}

void ILI9xxx::startWrite(void)
{
    m_cs.setLow();
}

void ILI9xxx::endWrite(void)
{
    m_cs.setHigh();
}

void ILI9xxx::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    startWrite();

    writePixel(x, y, color);

    endWrite();
}

void ILI9xxx::writePixel(int16_t x, int16_t y, uint16_t color)
{
    setAddrWindow(x, y, 1, 1);
    writePixels(&color, 1);
}

void ILI9xxx::writePixels(uint16_t * colors, uint32_t len)
{
    m_spi.write(reinterpret_cast<uint8_t*>(colors), len*2, SPI_MAX_FREQUENCY);
}

bool ILI9xxx::drawBmpFromFile(File& file, int32_t x, int32_t y, bool invert)
{
    /* Outside the screen */
    if(x >= m_size.x() ||
       y >= m_size.y())
       return false;

    /* Too small to be a BMP file */
    if(file.size() < 54)
        return true;

    ByteArray buff;
    
    if(file.rewind())
        return true;

    /* Read header and DIB */
    /* See Wikipedia for BMP format details */
    if(file.read(buff, 54) || buff.size() != 54)
        return true;

    /* Check magic constant */
    if(! (  buff[0] == 0x42  &&
            buff[1] == 0x4D))
        return true;

    /* Pixel array offset field */
    uint32_t pixelArrayOffset =   (uint32_t)(buff[0x0A])        | ((uint32_t)(buff[0x0B])<<8) |
                                 ((uint32_t)(buff[0x0C])<<16)   | ((uint32_t)(buff[0x0D])<<24);
    
    /* Offset is beyond EOF */
    if(pixelArrayOffset >= file.size() || file.seek(pixelArrayOffset))
        return true;


    /* Check color depth (only 16bits is accepted) */
    uint16_t colorDepth = (uint16_t)(buff[0x1C]) | ((uint16_t)(buff[0x1D])<<8);
    if(colorDepth != 16)
        return true;

    uint32_t width =         (uint32_t)(buff[0x12])         | ((uint32_t)(buff[0x13])<<8) |
                            ((uint32_t)(buff[0x14])<<16)    | ((uint32_t)(buff[0x15])<<24);

    uint32_t height=         (uint32_t)(buff[0x16])         | ((uint32_t)(buff[0x17])<<8) |
                            ((uint32_t)(buff[0x18])<<16)    | ((uint32_t)(buff[0x19])<<24);

    /* Opposite corner coordinates */
    int32_t x2 = x+width-1;
    int32_t y2 = y+height-1;

    /* Outside the screen */
    if(x2 <= 0 || y2 <= 0)
        return false;

    uint32_t rh = height;
    uint32_t rw = width;

    /* Clip for partial image */
    if(y2 >= m_size.y())
        rh = m_size.y() - y;

    if(x2 >= m_size.x())
        rw = m_size.x() - x;

    if(x < 0)
        rw = width + x;

    bool error = false;
    const uint32_t rowSize = width*sizeof(uint16_t) + (width%2)*sizeof(uint16_t);


    uint16_t *row = reinterpret_cast<uint16_t*>(malloc(rowSize));

    startWrite();
    
    if(!invert)
        setAddrWindow(x>0?x:0, y>0?y:0, rw, rh);

    if(y < 0 && !invert)
        if(file.seek(pixelArrayOffset + (-y)*rowSize))
            return true;

    if(invert && height > rh)
        if(file.seek(pixelArrayOffset + (height - rh)*rowSize))
            return true;

    for(uint32_t line = (y>0)?0:(-y) ; line < rh; ++line)
    {
        if(file.read(buff, rowSize) || buff.size() != rowSize)
        {
            error = true;
            break;
        }

        /* A copy is necessary because of endianess issues */
        for(uint32_t i=0, k=0; i<width; ++i, k += sizeof(uint16_t))
            row[i] = ((uint16_t)(buff[k])<<8) | (uint16_t)(buff[k+1]);

        if(invert)
            setAddrWindow(x>0?x:0, (y>0?y:0) + (rh - line), rw, 1);
            
        writePixels(x>=0?row:(uint16_t*)(row-x), rw);
    }

    endWrite();

    free(row);

    return error;
}

void ILI9xxx::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    if((x >= m_size.x()) || (y >= m_size.y()))
        return;

    int16_t x2 = x + w - 1, y2 = y + h - 1;

    if((x2 < 0) || (y2 < 0))
        return;

    /* Clip left/top */
    if(x < 0)
    {
        x = 0;
        w = x2 + 1;
    }
    if(y < 0)
    {
        y = 0;
        h = y2 + 1;
    }

    /* Clip right/bottom */
    if(x2 >= m_size.x())
        w = m_size.x()  - x;

    if(y2 >= m_size.y())
        h = m_size.y() - y;

    setAddrWindow(x, y, w, h);

    uint32_t len = w*h;
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

/* This code was ported/adapted from https://github.com/PaulStoffregen/ILI9341_t3 */
/* by Marc MERLIN. See examples/pictureEmbed to use this. */
void ILI9xxx::drawImage(    int16_t x, int16_t y,
                            int16_t w, int16_t h,
                            const uint16_t *pcolors)
{

    int16_t x2, y2; // Lower-right coord
    if(( x             >= m_size.x()) ||       // Off-edge right
       ( y             >= m_size.y()) ||       // " top
       ((x2 = (x+w-1)) <  0      ) ||       // " left
       ((y2 = (y+h-1)) <  0)     )          // " bottom
       return; 

    int16_t bx1=0, by1=0, // Clipped top-left within bitmap
            saveW=w;      // Save original bitmap width value

    if(x < 0)
    { // Clip left
        w  +=  x;
        bx1 = -x;
        x   =  0;
    }
    if(y < 0)
    { // Clip top
        h  +=  y;
        by1 = -y;
        y   =  0;
    }

    if(x2 >= m_size.x())
        w = m_size.x()  - x; // Clip right

    if(y2 >= m_size.y())
        h = m_size.y() - y; // Clip bottom

    pcolors += by1 * saveW + bx1; // Offset bitmap ptr to clipped top-left
    startWrite();
    setAddrWindow(x, y, w, h); // Clipped area
    while(h--)
    { // For each (clipped) scanline...
      writePixels((uint16_t*)pcolors, w); // Push one (clipped) row
      pcolors += saveW; // Advance pointer by one full (unclipped) line
    }
    endWrite();
}

