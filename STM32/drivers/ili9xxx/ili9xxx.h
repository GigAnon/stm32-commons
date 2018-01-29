#ifndef GUARD_ILI9XXX
#define GUARD_ILI9XXX

#include "display.h"
#include "spi.h"
#include "pin.h"

#define ILI9xxx_USE_FILESYSTEM 1
#if ILI9xxx_USE_FILESYSTEM
    #include "filesystem/file.h"
#endif

class ILI9xxx: public Display
{
    public:
        ILI9xxx(uint16_t width, uint16_t height,
                SPI& spi, Pin cs, Pin dc, Pin rst = Pin());

        virtual void drawPixel(int16_t x, int16_t y, uint16_t color);

        virtual void drawImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *pcolors);

        #if ILI9xxx_USE_FILESYSTEM
        /*
            \brief Stream a BMP-565 from the provided file handler, and display it.
            \param file Image file to be displayed
            \param x X coordinate for upper corner
            \param y Y coordinate for upper corner
            \returns \c true on error, \c false otherwise
        */
        bool drawBmpFromFile(File& file, int32_t x, int32_t y, bool invert = true);
    #endif

    protected:

        virtual bool init(Orientation orientation = Orientation::PORTRAIT_1);
        virtual void reset();

        virtual void startWrite(void);
        virtual void endWrite(void);

        virtual void writePixel(int16_t x, int16_t y, uint16_t color);

        virtual void writePixels(uint16_t * colors, uint32_t len);

        virtual void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) = 0;

        virtual void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
        
        SPI& m_spi;
        Pin m_cs;
        Pin m_dc;
        Pin m_rst;

        static constexpr uint32_t SPI_MAX_PIXELS_AT_ONCE = 64;
        static constexpr uint32_t SPI_MAX_FREQUENCY = 36000000;

    private:
};

#endif
