#include "color.h"

const Color16 Color16::Black    = Color16(0x00, 0x00, 0x00);
const Color16 Color16::White    = Color16(0x31, 0xFF, 0xFF);
const Color16 Color16::Red      = Color16(0x31, 0x00, 0x00);
const Color16 Color16::Green    = Color16(0x00, 0xFF, 0x00);
const Color16 Color16::Blue     = Color16(0x00, 0x00, 0xFF);

Color16::Color16(uint8_t r, uint8_t g, uint8_t b)
{
    m_color = toUInt16Rgb(r, g, b);
}

Color16::Color16(uint16_t colorRgb)
{
    m_color = colorRgb;
}

uint16_t Color16::toUInt16Rgb(uint8_t r, uint8_t g, uint8_t b)
{
    /*uint16_t ra = (r&(0xF8))>>3;
    uint16_t ga = (g&(0xFC))>>2;
    uint16_t ba = (b&(0xF8))>>3;

    uint16_t c = (ra<<11)|(ga<<5)|ba;

    return (c>>8) | ((c&0xFF)<<8);*/

    return ((((r)& 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3));
}

uint16_t Color16::toUInt16() const
{
    return m_color;
}

uint8_t Color16::red() const
{
    return (m_color&0xF800)>>11;
}

uint8_t Color16::green() const
{
    return (m_color&0x07E0)>>5;
}

uint8_t Color16::blue() const
{
    return (m_color&0x1F);
}
