#ifndef GUARD_SCREEN
#define GUARD_SCREEN

#include "gfxfont.h"

#include "color.h"
#include "point2d.h"

#include <cstdint>

/**
    \brief Base abstract class for screen displays.

    \todo Refactor and document
    \remark Based on Adafruit's excellent Adafruit_GFX class
**/
class Display
{
    public:
        
        Display(int16_t w, int16_t h);
        virtual ~Display() {}

        virtual bool init(Orientation orientation) = 0;

        virtual void writePixel(int16_t x, int16_t y, uint16_t color) = 0;

        virtual void drawPixel(int16_t x, int16_t y, uint16_t color);

        virtual void writeFillRect(     int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
        virtual void writeFastVLine(    int16_t x, int16_t y, int16_t h, uint16_t color);
        virtual void writeFastHLine(    int16_t x, int16_t y, int16_t w, uint16_t color);
        virtual void writeLine(         int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

        virtual void startWrite();
        virtual void endWrite();

        // CONTROL API
        // These MAY be overridden by the subclass to provide device-specific
        // optimized code.  Otherwise 'generic' versions are used.
        virtual void invertDisplay(bool i);

        // BASIC DRAW API
        // These MAY be overridden by the subclass to provide device-specific
        // optimized code.  Otherwise 'generic' versions are used.
        virtual void drawFastVLine( int16_t x, int16_t y, int16_t h, uint16_t color);
        virtual void drawFastHLine( int16_t x, int16_t y, int16_t w, uint16_t color);
        virtual void fillRect(      int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
        virtual void fillScreen(    uint16_t color);

        // Optional and probably not necessary to change
        virtual void drawLine(  int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
        virtual void drawRect(  int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

      // These exist only with Adafruit_GFX (no subclass overrides)
        void drawCircle(        int16_t x0, int16_t y0, int16_t r, uint16_t color);
        void drawCircleHelper(  int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
                                uint16_t color);

        void fillCircle(        int16_t x0, int16_t y0, int16_t r, uint16_t color);

        void fillCircleHelper(  int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
                                int16_t delta, uint16_t color);

        void drawTriangle(  int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                            int16_t x2, int16_t y2, uint16_t color);

        void fillTriangle(  int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                            int16_t x2, int16_t y2, uint16_t color);

        void drawRoundRect( int16_t x0, int16_t y0, int16_t w, int16_t h,
                            int16_t radius, uint16_t color);

        void fillRoundRect( int16_t x0, int16_t y0, int16_t w, int16_t h,
                            int16_t radius, uint16_t color);

        void drawBitmap(    int16_t x, int16_t y, const uint8_t *bitmap,
                            int16_t w, int16_t h, uint16_t color);

        void drawBitmap(    int16_t x, int16_t y, const uint8_t *bitmap,
                            int16_t w, int16_t h, uint16_t color, uint16_t bg);

        void drawBitmap(    int16_t x, int16_t y, uint8_t *bitmap,
                            int16_t w, int16_t h, uint16_t color);

        void drawBitmap(    int16_t x, int16_t y, uint8_t *bitmap,
                            int16_t w, int16_t h, uint16_t color, uint16_t bg);

        void drawXBitmap(   int16_t x, int16_t y, const uint8_t *bitmap,
                            int16_t w, int16_t h, uint16_t color);

        void drawChar(      int16_t x, int16_t y, unsigned char c, uint16_t color,
                            uint16_t bg, uint8_t size);

        void setCursor(int16_t x, int16_t y);
        void setTextColor(uint16_t c);
        void setTextColor(uint16_t c, uint16_t bg);
        void setTextSize(uint8_t s);
        void setTextWrap(bool w);
        void cp437(bool x=true);
        void setFont(const GFXfont *f = nullptr);

        void getTextBounds(char *string, int16_t x, int16_t y,
          int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);

        void getTextBounds(const char *s, int16_t x, int16_t y,
          int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);

        virtual std::size_t write(uint8_t);

        void write(const char* str);

        int16_t height() const;
        int16_t width() const;

        Point2D size() const;

        virtual void setOrientation(Orientation orientation);
        Orientation orientation() const;

        // get current cursor position (get rotation safe maximum values, using: width() for x, height() for y)
        int16_t getCursorX() const;
        int16_t getCursorY() const;

        uint32_t doBenchmark();

    protected:
        const int16_t WIDTH;
        const int16_t HEIGHT;   // This is the 'raw' display w/h - never changes

        void setSize(Point2D s);
        virtual Point2D orient(const Point2D& p);

        Point2D m_size;

        int16_t     cursor_x    = 0;
        int16_t     cursor_y    = 0;

        uint16_t    textcolor   = 0xFFFF;
        uint16_t    textbgcolor = 0xFFFF;

        uint8_t     textsize    = 1;

        Orientation m_orientation = Orientation::LANDSCAPE_1;

        bool        wrap    = true;   // If set, 'wrap' text at right edge of display
        bool        _cp437  = false; // If set, use correct CP437 charset (default is off)
        GFXfont*    gfxFont = nullptr;
};

#endif
