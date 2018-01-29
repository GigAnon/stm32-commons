#ifndef GUARD_SSD1306
#define GUARD_SSD1306

#include "pin.h"
#include "spi.h"
#include "i2c.h"

/*=========================================================================
    SSD1306 Displays
    -----------------------------------------------------------------------
    The driver is used in multiple displays (128x64, 128x32, etc.).
    Select the appropriate display below to create an appropriately
    sized framebuffer, etc.

    SSD1306_128_64  128x64 pixel display

    SSD1306_128_32  128x32 pixel display

    You also need to set the LCDWIDTH and LCDHEIGHT defines to an 
    appropriate size

    -----------------------------------------------------------------------*/
#define SSD1306_128_64
//#define SSD1306_128_32
/*=========================================================================*/

#if defined SSD1306_128_64 && defined SSD1306_128_32
  #error "Only one SSD1306 display can be specified at once in SSD1306.h"
#endif
#if !defined SSD1306_128_64 && !defined SSD1306_128_32
  #error "At least one SSD1306 display must be specified in SSD1306.h"
#endif

#if defined SSD1306_128_64
  #define SSD1306_LCDWIDTH                  128
  #define SSD1306_LCDHEIGHT                 64
#endif
#if defined SSD1306_128_32
  #define SSD1306_LCDWIDTH                  128
  #define SSD1306_LCDHEIGHT                 32
#endif

struct Segments
{
    enum Value
    {
        NONE        = 0,
        TOP         = 1<<1,
        TOP_LEFT    = 1<<2,
        TOP_RIGHT   = 1<<3,
        MIDDLE      = 1<<4,
        BOTTOM_LEFT = 1<<5,
        BOTTOM_RIGHT= 1<<6,
        BOTTOM      = 1<<7,
        ALL         = TOP|TOP_LEFT|TOP_RIGHT|MIDDLE|BOTTOM_LEFT|BOTTOM_RIGHT|BOTTOM,

        ZERO    = ALL^MIDDLE,
        ONE     = TOP_RIGHT|BOTTOM_RIGHT,
        TWO     = TOP|TOP_RIGHT|MIDDLE|BOTTOM_LEFT|BOTTOM,
        THREE   = TOP|MIDDLE|BOTTOM|TOP_RIGHT|BOTTOM_RIGHT,
        FOUR    = TOP_LEFT|TOP_RIGHT|MIDDLE|BOTTOM_RIGHT,
        FIVE    = TOP|TOP_LEFT|MIDDLE|BOTTOM_RIGHT|BOTTOM,
        SIX     = ALL^TOP_RIGHT,
        SEVEN   = TOP|TOP_RIGHT|BOTTOM_RIGHT,
        EIGTH   = ALL,
        NINE    = ALL^BOTTOM_LEFT
    };

    static Segments::Value intToSegments(uint8_t num);
};

/**
    \brief Class driver for SSD1306 OLED screen driver.

    Class driver for SSD1306 OLED screen driver.
    Can be used with 4-wires SPI or I2C. 
**/
class SSD1306
{
	public:
        /**
            \brief Power supply type enumeration

            Power supply can be external or internal (charge pump from Vdd)
        **/
        enum class SupplyType
        {
            External,
            Internal
        };

        /**
            \brief SPI 4-wires constructor
            \param spi SPI device the SSD1306 is connected to
            \param dc DC pin
            \param rst RST pin
            \param cs CS pin

            \remark SPI device will not be initialized by this method
        **/
		SSD1306(SPI& spi, Pin dc, Pin rst, Pin cs);

        /**
            \brief I2C constructor
            \param i2c I2C peripheral
            \param rst RST pin (default: Pin() for no reset pin)

            \remark I2C device will not be initialized by this method
            \remark SSD1306 I2C address is SSD1306::I2C_ADDRESS
        **/
        SSD1306(I2C& i2c, Pin rst = Pin());

        /**
            \brief Initialize SSD1306 driver
            \param switchvcc Power supply type (default: internal charge pump)
        **/
		void init(SSD1306::SupplyType switchvcc = SSD1306::SupplyType::Internal);

        /**
            \brief Set screen contrast
            \param val Contrast value [0-255]

            \remark Actual contrast values might be different from one board/screen to another.
        **/
		void setContrast(uint8_t val);

        /**
            \brief Clear screen

            \remark Equivalent to clear() followed by display()
        **/
		void clearDisplay();

        /**
            \brief Clear internal buffer

            \remark This method does not send the buffer to the screen driver
        **/
		void clear();

        /**
            \brief Invert pixels values on the screen
            \param inverted Set to \c true to invert colors, \c false otherwise
        **/
		void setColorInverted(bool inverted);

        /**
            \brief Invert display (set upside-down)
            \param inverted Set to \c true to invert display, \c false otherwise
        **/
        void setInverted(bool inverted);

        /**
            \brief Send the internal buffer to screen
        **/
		void display();

        /**
            \brief Set pixel in the internal buffer
            \param x X coordinate
            \param y Y coordinate
            \param color Pixel value
        **/
		void setPixel(uint8_t x, uint8_t y,                             uint8_t color = WHITE);

        /**
            \brief Draw a disc in the internal buffer
            \param x0 Center X coordinate
            \param y0 Center Y coordinate
            \param r Radius
            \param color Pixel value
        **/
		void fillCircle(uint8_t x0, uint8_t y0, uint8_t r,              uint8_t color = WHITE);

        /**
            \brief Draw a circle in the internal buffer
            \param x0 Center X coordinate
            \param y0 Center Y coordinate
            \param r Radius
            \param color Pixel value
        **/
		void drawCircle(uint8_t x0, uint8_t y0, uint8_t r,              uint8_t color = WHITE);

        /**
            \brief Draw a rectangle in the internal buffer
            \param x Corner X coordinate
            \param y Corner Y coordinate
            \param w Width
            \param h Heigth
            \param color Pixel value
        **/
		void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h,       uint8_t color = WHITE);

        /**
            \brief Draw a filled-in the rectangle in internal buffer
            \param x Corner X coordinate
            \param y Corner Y coordinate
            \param w Width
            \param h Heigth
            \param color Pixel value
        **/
		void fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h,       uint8_t color = WHITE);

        /**
            \brief Draw a line in the internal buffer
            \param x0 First point X coordinate
            \param y0 First point Y coordinate
            \param x1 Second point X coordinate
            \param y1 Second point Y coordinate
            \param color Pixel value
        **/
		void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,   uint8_t color = WHITE);

        /**
            \brief Draw a character in the internal buffer
            \param x X coordinate
            \param line Line
            \param c Character to draw
        **/
		void drawChar(uint8_t x, uint8_t line, uint8_t c);

        /**
            \brief Draw a string in the internal buffer
            \param x Start X coordinate
            \param line Start line
            \param c Nul-terminated c-string to write

            \remark If the end of a line is reached, the method will continue on the next
        **/
		void drawString(uint8_t x, uint8_t line, const char *c);

        void drawSevenSegments(uint8_t x, uint8_t y, uint8_t num, uint8_t size = 20);
        void drawSevenSegments(uint8_t x, uint8_t y, Segments::Value seg, uint8_t size = 20);
        void drawMultipleSegments(uint8_t x, uint8_t y, uint16_t num, uint8_t size = 20);

        /**
            \brief Draw a bitmat in the internal buffer
            \param x Start X coordinate
            \param y Start Y coordinate
            \param bitmap Bitmap buffer
            \param w Bitmap width
            \param h Bitmap height
            \param color Pixel value
        **/
		void drawBitmap(uint8_t x, uint8_t y,
				const uint8_t *bitmap, uint8_t w, uint8_t h,
				uint8_t color);

    public:
        static constexpr uint8_t BLACK      = 0; ///< Black (off) pixel
        static constexpr uint8_t WHITE      = 1; ///< White (on) pixel
        static constexpr uint8_t I2C_ADR    = 0x3C; ///< Device I2C address (when relevant)

    protected:
        /* SSD1306 commands */
        static constexpr uint8_t SETCONTRAST            = 0x81;
        static constexpr uint8_t DISPLAYALLON_RESUME    = 0xA4;
        static constexpr uint8_t DISPLAYALLON           = 0xA5;
        static constexpr uint8_t NORMALDISPLAY          = 0xA6;
        static constexpr uint8_t INVERTDISPLAY          = 0xA7;
        static constexpr uint8_t DISPLAYOFF             = 0xAE;
        static constexpr uint8_t DISPLAYON              = 0xAF;

        static constexpr uint8_t ACTIVATE_SCROLL        = 0x2F;
        static constexpr uint8_t DEACTIVATE_SCROLL      = 0x2E;

        static constexpr uint8_t MEMORYMODE             = 0x20;
        static constexpr uint8_t COLUMNADDR             = 0x21;
        static constexpr uint8_t PAGEADDR               = 0x22;

        static constexpr uint8_t SETDISPLAYOFFSET       = 0xD3;
        static constexpr uint8_t SETCOMPINS             = 0xDA;

        static constexpr uint8_t SETVCOMDETECT          = 0xDB;

        static constexpr uint8_t SETDISPLAYCLOCKDIV     = 0xD5;
        static constexpr uint8_t SETPRECHARGE           = 0xD9;

        static constexpr uint8_t SETMULTIPLEX           = 0xA8;

        static constexpr uint8_t SETLOWCOLUMN           = 0x00;
        static constexpr uint8_t SETHIGHCOLUMN          = 0x10;

        static constexpr uint8_t SETSTARTLINE           = 0x40;

        static constexpr uint8_t COMSCANINC             = 0xC0;
        static constexpr uint8_t COMSCANDEC             = 0xC8;

        static constexpr uint8_t SEGREMAP               = 0xA0;

        static constexpr uint8_t CHARGEPUMP             = 0x8D;

	private:
        I2C* m_i2c = nullptr;
		SPI* m_spi = nullptr;

        bool m_displayInverted = false;

		Pin m_dcPin;
		Pin m_rstPin;
		Pin m_csPin;

        void command(uint8_t c);
};

#endif
