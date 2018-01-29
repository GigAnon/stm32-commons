/*
This is the core graphics library for all our displays, providing a common
set of graphics primitives (points, lines, circles, etc.).  It needs to be
paired with a hardware-specific library for each display device we carry
(to handle the lower-level functions).

Adafruit invests time and resources providing this open source code, please
support Adafruit & open-source hardware by purchasing products from Adafruit!

Copyright (c) 2013 Adafruit Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
 */

#include "display.h"

#include <cmath>
#include <algorithm>
#include <cstdio>
#include "system.h"

extern const unsigned char glcdfont_font[];

Display::Display(int16_t w, int16_t h):
    WIDTH(w), HEIGHT(h)
{
    setSize(Point2D(WIDTH, HEIGHT));
}

Point2D Display::size() const
{
    return m_size;
}

void Display::setSize(Point2D s)
{
    m_size = s;
}

// Bresenham's algorithm - thx wikpedia
void Display::writeLine(int16_t x0, int16_t y0,
                        int16_t x1, int16_t y1,
                        uint16_t color)
{
    int16_t steep = std::abs(y1 - y0) > std::abs(x1 - x0);
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

    int16_t dx, dy;
    dx = x1 - x0;
    dy = std::abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep = (y0 < y1)?1:-1;

    for (; x0<=x1; ++x0)
    {
        if (steep)
            writePixel(y0, x0, color);
        else
            writePixel(x0, y0, color);

        err -= dy;
        if (err < 0)
        {
            y0 += ystep;
            err += dx;
        }
    }
}

void Display::startWrite()
{
    // Overwrite in subclasses if desired!
}

void Display::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    startWrite();
    writePixel(x, y, color);
    endWrite();
}

// (x,y) is topmost point; if unsure, calling function
// should sort endpoints or call writeLine() instead
void Display::writeFastVLine(  int16_t x, int16_t y,
                               int16_t h, uint16_t color)
{
    writeFillRect(x, y, 1, h, color);
}

// (x,y) is leftmost point; if unsure, calling function
// should sort endpoints or call writeLine() instead
void Display::writeFastHLine(  int16_t x, int16_t y,
                               int16_t w, uint16_t color)
{
    writeFillRect(x, y, w, 1, color);
}

void Display::writeFillRect(   int16_t x, int16_t y,
                               int16_t w, int16_t h,
                               uint16_t color)
{
    if( w <= 0 ||
        h <= 0)
        return;

    for(int32_t i = x; i < x+w; ++i)
        for(int32_t j = y; j < y+h; ++j)
            drawPixel(i, j, color);
}

void Display::endWrite()
{
    // Overwrite in subclasses if startWrite is defined!
}

// (x,y) is topmost point; if unsure, calling function
// should sort endpoints or call drawLine() instead
void Display::drawFastVLine(   int16_t x, int16_t y,
                               int16_t h, uint16_t color)
{
    // Update in subclasses if desired!
    startWrite();
    writeFastVLine(x, y, h, color);
    endWrite();
}

// (x,y) is leftmost point; if unsure, calling function
// should sort endpoints or call drawLine() instead
void Display::drawFastHLine(   int16_t x, int16_t y,
                               int16_t w, uint16_t color)
{
    // Update in subclasses if desired!
    startWrite();
    writeFastHLine(x, y, w, color);
    endWrite();
}

void Display::fillRect(int16_t x, int16_t y,
                       int16_t w, int16_t h,
                       uint16_t color)
{
    startWrite();
    writeFillRect(x, y, w, h, color);
    endWrite();
}

void Display::fillScreen(uint16_t color)
{
    // Update in subclasses if desired!
    fillRect(0, 0, m_size.x(), m_size.y(), color);
}

void Display::drawLine(    int16_t x0, int16_t y0,
                           int16_t x1, int16_t y1,
                           uint16_t color)
{
    // Update in subclasses if desired!
    if(x0 == x1)
    {
        if(y0 > y1)
            std::swap(y0, y1);

        drawFastVLine(x0, y0, y1 - y0 + 1, color);
    }
    else if(y0 == y1)
    {
        if(x0 > x1)
            std::swap(x0, x1);

        drawFastHLine(x0, y0, x1 - x0 + 1, color);
    }
    else
    {
        startWrite();
        writeLine(x0, y0, x1, y1, color);
        endWrite();
    }
}


// Draw a circle outline
void Display::drawCircle(  int16_t x0, int16_t y0, int16_t r,
                           uint16_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    startWrite();
    writePixel(x0  , y0+r, color);
    writePixel(x0  , y0-r, color);
    writePixel(x0+r, y0  , color);
    writePixel(x0-r, y0  , color);

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

        writePixel(x0 + x, y0 + y, color);
        writePixel(x0 - x, y0 + y, color);
        writePixel(x0 + x, y0 - y, color);
        writePixel(x0 - x, y0 - y, color);
        writePixel(x0 + y, y0 + x, color);
        writePixel(x0 - y, y0 + x, color);
        writePixel(x0 + y, y0 - x, color);
        writePixel(x0 - y, y0 - x, color);
    }
    endWrite();
}

void Display::drawCircleHelper(int16_t x0, int16_t y0,
                               int16_t r, uint8_t cornername,
                               uint16_t color)
{
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y)
    {
        if (f >= 0)
        {
            --y;
            ddF_y += 2;
            f     += ddF_y;
        }
        ++x;
        ddF_x += 2;
        f     += ddF_x;

        if (cornername & 0x4)
        {
            writePixel(x0 + x, y0 + y, color);
            writePixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2)
        {
            writePixel(x0 + x, y0 - y, color);
            writePixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8)
        {
            writePixel(x0 - y, y0 + x, color);
            writePixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1)
        {
            writePixel(x0 - y, y0 - x, color);
            writePixel(x0 - x, y0 - y, color);
        }
    }
}

void Display::fillCircle(  int16_t x0, int16_t y0,
                           int16_t r,  uint16_t color)
{
    startWrite();
    writeFastVLine(x0, y0-r, 2*r+1, color);
    fillCircleHelper(x0, y0, r, 3, 0, color);
    endWrite();
}

// Used to do circles and roundrects
void Display::fillCircleHelper(int16_t x0, int16_t y0, int16_t r,
                               uint8_t cornername, int16_t delta, uint16_t color)
{

    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y)
    {
        if (f >= 0)
        {
            --y;
            ddF_y += 2;
            f     += ddF_y;
        }
        ++x;
        ddF_x += 2;
        f     += ddF_x;

        if (cornername & 0x1)
        {
            writeFastVLine(x0+x, y0-y, 2*y+1+delta, color);
            writeFastVLine(x0+y, y0-x, 2*x+1+delta, color);
        }
        if (cornername & 0x2)
        {
            writeFastVLine(x0-x, y0-y, 2*y+1+delta, color);
            writeFastVLine(x0-y, y0-x, 2*x+1+delta, color);
        }
    }
}

// Draw a rectangle
void Display::drawRect(int16_t x, int16_t y,
                       int16_t w, int16_t h,
                       uint16_t color)
{
    startWrite();
    writeFastHLine(x, y, w, color);
    writeFastHLine(x, y+h-1, w, color);
    writeFastVLine(x, y, h, color);
    writeFastVLine(x+w-1, y, h, color);
    endWrite();
}

// Draw a rounded rectangle
void Display::drawRoundRect(int16_t x, int16_t y, int16_t w,
                            int16_t h, int16_t r, uint16_t color)
{
    // smarter version
    startWrite();
    writeFastHLine(x+r  , y    , w-2*r, color); // Top
    writeFastHLine(x+r  , y+h-1, w-2*r, color); // Bottom
    writeFastVLine(x    , y+r  , h-2*r, color); // Left
    writeFastVLine(x+w-1, y+r  , h-2*r, color); // Right
    // draw four corners
    drawCircleHelper(x+r    , y+r    , r, 1, color);
    drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
    drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
    drawCircleHelper(x+r    , y+h-r-1, r, 8, color);
    endWrite();
}

// Fill a rounded rectangle
void Display::fillRoundRect(int16_t x, int16_t y, int16_t w,
                            int16_t h, int16_t r, uint16_t color)
{
    // smarter version
    startWrite();
    writeFillRect(x+r, y, w-2*r, h, color);

    // draw four corners
    fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
    fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
    endWrite();
}

// Draw a triangle
void Display::drawTriangle( int16_t x0, int16_t y0,
                            int16_t x1, int16_t y1,
                            int16_t x2, int16_t y2,
                            uint16_t color)
{
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
}

// Fill a triangle
void Display::fillTriangle( int16_t x0, int16_t y0,
                            int16_t x1, int16_t y1,
                            int16_t x2, int16_t y2,
                            uint16_t color)
{

    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1)
    {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }
    if (y1 > y2)
    {
        std::swap(y2, y1);
        std::swap(x2, x1);
    }
    if (y0 > y1)
    {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }

    startWrite();
    if(y0 == y2)
    { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if(x1 < a)
            a = x1;
        else if(x1 > b)
            b = x1;

        if(x2 < a)
            a = x2;
        else if(x2 > b)
            b = x2;
        writeFastHLine(a, y0, b-a+1, color);
        endWrite();
        return;
    }

    int16_t dx01 = x1 - x0;
    int16_t dy01 = y1 - y0;
    int16_t dx02 = x2 - x0;
    int16_t dy02 = y2 - y0;
    int16_t dx12 = x2 - x1;
    int16_t dy12 = y2 - y1;
    int32_t sa   = 0;
    int32_t sb   = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if(y1 == y2)
        last = y1;   // Include y1 scanline
    else
        last = y1-1; // Skip it

    for(y=y0; y<=last; ++y)
    {
        a   = x0 + sa / dy01;
        b   = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
         */
        if(a > b)
            std::swap(a,b);

        writeFastHLine(a, y, b-a+1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for(; y<=y2; ++y)
    {
        a   = x1 + sa / dy12;
        b   = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
         */
        if(a > b)
            std::swap(a,b);

        writeFastHLine(a, y, b-a+1, color);
    }
    endWrite();
}

// drawBitmap() variant for RAM-resident (not PROGMEM) bitmaps.
void Display::drawBitmap(int16_t x, int16_t y,
        uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{

    int16_t i, j, byteWidth = (w + 7) / 8;
    uint8_t byte = 0;

    startWrite();
    for(j=0; j<h; j++) {
        for(i=0; i<w; i++ ) {
            if(i & 7) byte <<= 1;
            else      byte   = bitmap[j * byteWidth + i / 8];
            if(byte & 0x80) writePixel(x+i, y+j, color);
        }
    }
    endWrite();
}

// drawBitmap() variant w/background for RAM-resident (not PROGMEM) bitmaps.
void Display::drawBitmap(int16_t x, int16_t y,
        uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg) {

    int16_t i, j, byteWidth = (w + 7) / 8;
    uint8_t byte = 0;

    startWrite();
    for(j=0; j<h; ++j)
    {
        for(i=0; i<w; ++i)
        {
            if(i & 7)
                byte <<= 1;
            else
                byte   = bitmap[j * byteWidth + i / 8];

            if(byte & 0x80)
                writePixel(x+i, y+j, color);
            else 
                writePixel(x+i, y+j, bg);
        }
    }
    endWrite();
}

//Draw XBitMap Files (*.xbm), exported from GIMP,
//Usage: Export from GIMP to *.xbm, rename *.xbm to *.c and open in editor.
//C Array can be directly used with this function
void Display::drawXBitmap(  int16_t x, int16_t y,
                            const uint8_t *bitmap,
                            int16_t w, int16_t h, uint16_t color)
{

    int16_t i, j, byteWidth = (w + 7) / 8;
    uint8_t byte = 0;

    startWrite();
    for(j=0; j<h; ++j)
    {
        for(i=0; i<w; ++i)
        {
            if(i & 7)
                byte >>= 1;
            else 
                byte = bitmap[j * byteWidth + i / 8];

            if(byte & 0x01)
                writePixel(x+i, y+j, color);
        }
    }
    endWrite();
}

// Draw a character
void Display::drawChar( int16_t x, int16_t y, unsigned char c,
                        uint16_t color, uint16_t bg, uint8_t size)
{
    if(!gfxFont)
    { // 'Classic' built-in font

        if(     (x >= m_size.x())            || // Clip right
                (y >= m_size.y())           || // Clip bottom
                ((x + 6 * size - 1) < 0) || // Clip left
                ((y + 8 * size - 1) < 0))   // Clip top
            return;

        if(!_cp437 && (c >= 176))
            ++c; // Handle 'classic' charset behavior

        startWrite();
        for(int8_t i=0; i<6; ++i)
        {
            uint8_t line;
            if(i < 5)
                line = glcdfont_font[(c*5)+i];
            else
                line = 0x0;

            for(int8_t j=0; j<8; ++j, line >>= 1)
            {
                if(line & 0x1)
                {
                    if(size == 1)
                        writePixel(x+i, y+j, color);
                    else
                        writeFillRect(x+(i*size), y+(j*size), size, size, color);
                }
                else if(bg != color)
                {
                    if(size == 1)
                        writePixel(x+i, y+j, bg);
                    else
                        writeFillRect(x+i*size, y+j*size, size, size, bg);
                }
            }
        }
        endWrite();

    }
    else
    { // Custom font

        // Character is assumed previously filtered by write() to eliminate
        // newlines, returns, non-printable characters, etc.  Calling drawChar()
        // directly with 'bad' characters of font may cause mayhem!

        c -= gfxFont->first;
     /*   GFXglyph *glyph  = &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
        uint8_t  *bitmap = (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);*/

        GFXglyph *glyph  = (GFXglyph *)(gfxFont->glyph+c);
        uint8_t  *bitmap = gfxFont->bitmap;

        uint16_t bo = glyph->bitmapOffset;
        uint8_t w  = glyph->width;
        uint8_t h  = glyph->height;
        int8_t  xo = glyph->xOffset;
        int8_t  yo = glyph->yOffset;
        uint8_t bits = 0;
        uint8_t bit = 0;
        int16_t xo16 = 0;
        int16_t yo16 = 0;

        if(size > 1)
        {
            xo16 = xo;
            yo16 = yo;
        }

        // Todo: Add character clipping here

        // NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
        // THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
        // has typically been used with the 'classic' font to overwrite old
        // screen contents with new data.  This ONLY works because the
        // characters are a uniform size; it's not a sensible thing to do with
        // proportionally-spaced fonts with glyphs of varying sizes (and that
        // may overlap).  To replace previously-drawn text when using a custom
        // font, use the getTextBounds() function to determine the smallest
        // rectangle encompassing a string, erase the area with fillRect(),
        // then draw new text.  This WILL infortunately 'blink' the text, but
        // is unavoidable.  Drawing 'background' pixels will NOT fix this,
        // only creates a new set of problems.  Have an idea to work around
        // this (a canvas object type for MCUs that can afford the RAM and
        // displays supporting setAddrWindow() and pushColors()), but haven't
        // implemented this yet.

        startWrite();
        for(uint8_t yy=0; yy<h; ++yy)
        {
            for(uint8_t xx=0; xx<w; ++xx)
            {
                if(!(bit++ & 7))
                    bits = bitmap[bo++];
                
                if(bits & 0x80)
                {
                    if(size == 1)
                        writePixel(x+xo+xx, y+yo+yy, color);
                    else
                        writeFillRect(x+(xo16+xx)*size, y+(yo16+yy)*size, size, size, color);
                }
                bits <<= 1;
            }
        }
        endWrite();

    } // End classic vs custom font
}

size_t Display::write(uint8_t c)
{
    if(c == '\n')
    {
        cursor_y += textsize*8;
        cursor_x  = 0;
    }
    else if(c == '\r')
    {
        // skip em
    }
    else
    {
        if(wrap && ((cursor_x + textsize * 6) >= m_size.x()))
        { // Heading off edge?
            cursor_x  = 0;            // Reset x to zero
            cursor_y += textsize * 8; // Advance y one line
        }
        drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);

        if(gfxFont)
            cursor_x += textsize * (gfxFont->glyph[c-gfxFont->first].width+1);
        else
            cursor_x += textsize * 6;
    }

    return 1;
}

void Display::write(const char* str)
{
    while(*str)
        write(*(str++));
}

void Display::setCursor(int16_t x, int16_t y)
{
    cursor_x = x;
    cursor_y = y;
}

int16_t Display::getCursorX(void) const
{
    return cursor_x;
}

int16_t Display::getCursorY(void) const
{
    return cursor_y;
}

void Display::setTextSize(uint8_t s)
{
    textsize = (s > 0) ? s : 1;
}

void Display::setTextColor(uint16_t c)
{
    // For 'transparent' background, we'll set the bg
    // to the same as fg instead of using a flag
    textcolor = textbgcolor = c;
}

void Display::setTextColor(uint16_t c, uint16_t b)
{
    textcolor   = c;
    textbgcolor = b;
}

void Display::setTextWrap(bool w)
{
    wrap = w;
}

void Display::setOrientation(Orientation orientation)
{
    m_orientation = orientation;

    switch(m_orientation)
    {
        case Orientation::PORTRAIT_1:
        case Orientation::PORTRAIT_2:
            m_size = Point2D(WIDTH, HEIGHT);
            break;
        case Orientation::LANDSCAPE_1:
        case Orientation::LANDSCAPE_2:
            m_size = Point2D(HEIGHT, WIDTH);
            break;
        default:
            break;
    }
}

Orientation Display::orientation() const
{
    return m_orientation;
}

// Enable (or disable) Code Page 437-compatible charset.
// There was an error in glcdfont.c for the longest time -- one character
// (#176, the 'light shade' block) was missing -- this threw off the index
// of every character that followed it.  But a TON of code has been written
// with the erroneous character indices.  By default, the library uses the
// original 'wrong' behavior and old sketches will still work.  Pass 'true'
// to this function to use correct CP437 character values in your code.
void Display::cp437(bool x)
{
    _cp437 = x;
}

void Display::setFont(const GFXfont *f)
{
    if(f)
    {          // Font struct pointer passed in?
        if(!gfxFont)
        { // And no current font struct?
            // Switching from classic to new font behavior.
            // Move cursor pos down 6 pixels so it's on baseline.
            cursor_y += 6;
        }
    }
    else if(gfxFont)
    { // NULL passed.  Current font struct defined?
        // Switching from new to classic font behavior.
        // Move cursor pos up 6 pixels so it's at top-left of char.
        cursor_y -= 6;
    }
    gfxFont = (GFXfont *)f;
}

// Pass string and a cursor position, returns UL corner and W,H.
void Display::getTextBounds(char *str, int16_t x, int16_t y,
                            int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
{
    uint8_t c; // Current character

    *x1 = x;
    *y1 = y;
    *w  = *h = 0;

    uint16_t lineWidth = 0, maxWidth = 0; // Width of current, all lines

    while((c = *str++))
    {
        if(c != '\n')
        { // Not a newline
            if(c != '\r')
            { // Not a carriage return, is normal char
                if(wrap && ((x + textsize * 6) >= m_size.x()))
                {
                    x  = 0;            // Reset x to 0
                    y += textsize * 8; // Advance y by 1 line
                    if(lineWidth > maxWidth) maxWidth = lineWidth; // Save widest line
                    lineWidth  = textsize * 6; // First char on new line
                }
                else
                { // No line wrap, just keep incrementing X
                    lineWidth += textsize * 6; // Includes interchar x gap
                }
            } // Carriage return = do nothing
        }
        else
        { // Newline
            x  = 0;            // Reset x to 0
            y += textsize * 8; // Advance y by 1 line

            if(lineWidth > maxWidth)
                maxWidth = lineWidth; // Save widest line

            lineWidth = 0;     // Reset lineWidth for new line
        }
    }
    // End of string
    if(lineWidth)
        y += textsize * 8; // Add height of last (or only) line

    if(lineWidth > maxWidth)
        maxWidth = lineWidth; // Is the last or only line the widest?

    *w = maxWidth - 1;               // Don't include last interchar x gap
    *h = y - *y1;
}

// Same as above, but for PROGMEM strings
void Display::getTextBounds(const char *str,
                            int16_t x, int16_t y,
                            int16_t *x1, int16_t *y1,
                            uint16_t *w, uint16_t *h)
{
    uint8_t *s = (uint8_t *)str, c;

    *x1 = x;
    *y1 = y;
    *w  = *h = 0;
    
    uint16_t lineWidth = 0, maxWidth = 0; // Width of current, all lines

    while((c = (*s++)))
    {
        if(c != '\n')
        { // Not a newline
            if(c != '\r')
            { // Not a carriage return, is normal char
                if(wrap && ((x + textsize * 6) >= m_size.x()))
                {
                    x  = 0;            // Reset x to 0
                    y += textsize * 8; // Advance y by 1 line

                    if(lineWidth > maxWidth)
                        maxWidth = lineWidth; // Save widest line

                    lineWidth  = textsize * 6; // First char on new line
                }
                else
                { // No line wrap, just keep incrementing X
                    lineWidth += textsize * 6; // Includes interchar x gap
                }
            } // Carriage return = do nothing
        }
        else
        { // Newline
            x  = 0;            // Reset x to 0
            y += textsize * 8; // Advance y by 1 line

            if(lineWidth > maxWidth)
                maxWidth = lineWidth; // Save widest line

            lineWidth = 0;     // Reset lineWidth for new line
        }
    }
    // End of string
    if(lineWidth)
        y += textsize * 8; // Add height of last (or only) line

    if(lineWidth > maxWidth)
        maxWidth = lineWidth; // Is the last or only line the widest?

    *w = maxWidth - 1;               // Don't include last interchar x gap
    *h = y - *y1;
}

// Return the size of the display (per current rotation)
int16_t Display::width() const
{
    return m_size.x();
}

int16_t Display::height() const
{
    return m_size.y();
}

void Display::invertDisplay(bool i)
{
    // Do nothing, must be subclassed if supported by hardware
}

Point2D Display::orient(const Point2D& p)
{
    switch(orientation())
    {
        case Orientation::LANDSCAPE_1:
            return Point2D(p.y(), size().x() - p.x() - 1);
        case Orientation::LANDSCAPE_2:
            return Point2D(size().y() - p.y() - 1, p.x());
        case Orientation::PORTRAIT_2:
            return size() - p - Point2D(1, 1);
        default:
            return p;
    }
}


uint32_t Display::doBenchmark()
{
    uint32_t start = System::millis();

    const uint32_t sx = size().x();
    const uint32_t sy = size().y();

    for(uint32_t k = 0, km = 300; k<km; ++k)
    {
        const uint32_t thd = sx/3;

        volatile uint16_t r = Color16::Red;
        volatile uint16_t g = Color16::Green;
        volatile uint16_t b = Color16::Blue;

        fillRect(0,     0, thd, sy, Color16::Red);
        fillRect(thd,   0, thd, sy, Color16::Green);
        fillRect(thd*2, 0, thd, sy, Color16::Blue);
        
        drawLine(sx/2, sy/2, sx/2 + 50*cos(2*3.14*(float)(k*5)/(float)(km)), sy/2 + 50*sin(2*3.14*(float)(k*5)/(float)(km)), Color16::Black);
    }
    
    fillScreen(Color16::Black);

    for(uint32_t k = 0, km = 3000, sm = std::min(sx, sy)/2; k<km; ++k)
        drawLine(sx/2, sy/2, (sx/2) + sm*cos(2*3.14*(float)(k)/(float)(km)), sy/2 + sm*sin(2*3.14*(float)(k)/(float)(km)), Color16::White);

    for(uint32_t k = 0, km = 3000, sm = std::min(sx, sy)/2; k<km; ++k)
        drawLine(sx/2, sy/2, (sx/2) + sm*cos(-2*3.14*(float)(k)/(float)(km)), sy/2 + sm*sin(-2*3.14*(float)(k)/(float)(km)), Color16::Black);

    fillScreen(Color16::Black);

    drawFastHLine(0,    sy/2,   sx, Color16::Blue);
    drawFastVLine(sx/2, 0,      sy, Color16::Blue);

    uint32_t buf[sx];
    for (uint32_t i=1, x=1; i<(sx*400); ++i) 
    {
        ++x;
        if (x==sx)
            x=1;

        if (i>sx)
        {
            if ((x==sx/2) || (buf[x-1]==sy/2))
                drawPixel(x, buf[x-1], Color16::Blue);
            else
                drawPixel(x, buf[x-1], 0);
        }

        int y= sy/2 + sin(i*(6.28)/(sx*0.98)) * (sy/2 - 20) * sin(i*0.0001);
        drawPixel(x, y, 0xFFFF);
        buf[x-1]=y;
    }
    
    uint16_t colors[] = {Color16::Red, Color16::Blue, Color16::Green};
    fillScreen(0x0000);
    
    for(int k=0; k<15; ++k)
    {
        for (uint32_t i=15; i<sy; i+=5)
            drawLine(0, i, (i*1.44)-10, sy, colors[(k)%3]);

        for (uint32_t i=sy; i>15; i-=5)
            drawLine(sx, i, (i*1.44)-12, 15, colors[(k+1)%3]);

        for (uint32_t i=sy; i>15; i-=5)
            drawLine(1, i, sx-8-(i*1.44), 15, colors[(k+2)%3]);

        for (uint32_t i=15; i<sy; i+=5)
            drawLine(sx, i, sx-9-(i*1.44), sy, colors[(k)%3]);
    }
    
    const char lorem[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque accumsan est et nunc tristique tincidunt. In ultrices velit et vehicula fermentum. Sed lobortis id" \
                         "mi sed mattis. Nulla non imperdiet ipsum. Nam diam nisi, fringilla a libero non, hendrerit iaculis augue. In congue nunc dui, sed maximus ligula faucibus nec." \
                         "Praesent feugiat mattis diam non tempus. Pellentesque porttitor commodo enim laoreet ullamcorper. Ut non risus lorem. Fusce vulputate tristique lacinia. In a mattis ex." \
                         "Cras neque orci, egestas at finibus id, laoreet quis eros. Integer tincidunt consequat diam, non facilisis arcu. Nunc lacinia, lectus a viverra volutpat, sem nulla" \
                         "interdum lectus, nec imperdiet arcu tortor at arcu. Donec eros ipsum, sollicitudin ut metus quis, semper vulputate libero";

    for(uint32_t i=0;i<25;++i)
    {
        fillScreen(0x0000);
        setCursor(0, 0);
        write(lorem);
    }
    
    fillScreen(0xFFFF);

    const int steps = std::min(sx, sy)/10;
    const float xstep = (float)sx/(float)(steps*2);
    const float ystep = (float)sy/(float)(steps*2);

    for(int c=0;c<30;++c)
    {
        float x=xstep, y=ystep;

        for(int i=0; i<steps; ++i, x+=xstep, y+=ystep)
        {
            fillRect(0,     0,      x, y, colors[c%3]);
            fillRect(0,     sy-y,   x, y, colors[c%3]);
            fillRect(sx-x,  sy-y,   x, y, colors[c%3]);
            fillRect(sx-x,  0,      x, y, colors[c%3]);
        }
    }
    
    uint32_t l = System::millis()-start;

    char s[32] = {0};

    snprintf(s, sizeof(s), "Time: %ld ms", l);
    
    fillScreen(0x0000);
    setCursor(25, sy/3*2);
    write(s);

    return l;
}
