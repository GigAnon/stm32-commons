#ifndef GUARD_ILI9225
#define GUARD_ILI9225

#include "../ili9xxx/ili9xxx.h"
#include "spi.h"
#include "pin.h"

/**
    \brief Driver for ILI9225-based screens

    Driver for ILI9225 screen driver, using 4-wires SPI interface.
**/
class ILI9225: public ILI9xxx
{
        static constexpr uint32_t HEIGHT    = 176; ///< Screen height
        static constexpr uint32_t WIDTH     = 220; ///< Screen width

    public:
        /**
            \brief Constructor
            \param spi SPI interface to be used
            \param cs CS pin
            \param dc DC pin
            \param rst RESET pin
        **/
        ILI9225(SPI& spi, Pin cs, Pin dc, Pin rst = Pin());

        /**
            \brief Destructor
        **/
        virtual ~ILI9225() {}

        /**
            \brief Initialize the driver
            \param orientation Orientation of the screen
            \return \c true on error, \c false otherwise

            \remark The SPI interface must have been prealably initialized.
        **/
        virtual bool init(Orientation orientation = Orientation::LANDSCAPE_2);

        /**
            \brief Set screen orientation
            \param orientation Orientation of the screen
        **/
        virtual void setOrientation(Orientation orientation);

        void fillRect( int16_t x, int16_t y,
                       int16_t w, int16_t h,
                       uint16_t color);

    private:
        
        virtual void writeFillRect(int16_t x, int16_t y,  int16_t w, int16_t h, uint16_t color);
        virtual inline void writeFastVLine (int16_t x, int16_t y,  int16_t h, uint16_t color) { writeFillRect(x, y, 1, h, color); }
        virtual inline void writeFastHLine (int16_t x, int16_t y,  int16_t w, uint16_t color) { writeFillRect(x, y, w, 1, color); }

        void writeCommand(uint8_t reg, uint16_t val);
        void writeCommand(uint8_t reg);

        void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

        /**
            ENTRY_MODE configuration parameters
        **/
        enum EntryModeConfig
        {
            EM_BGR    = 1<<12,
            EM_MDT1   = 1<< 9,
            EM_MDT0   = 1<< 8,
            EM_ID1    = 1<< 5,
            EM_ID0    = 1<< 4,
            EM_AM     = 1<< 3,
        };

        uint16_t m_entryMode;

        /**
            ILI9225 commands
        **/
        enum Commands
        {
            CHIP_ID                 = 0x00,
            OUTPUT_CTRL             = 0x01,
            LCD_AC_CTRL             = 0x02,
            ENTRY_MODE              = 0x03,

            DISPLAY_CTRL            = 0x07,
            BLANK_PERIOD            = 0x08,

            FRAME_CYCLE_CTRL        = 0x0B,
            INTERFACE_CTRL          = 0x0C,

            OSC_CTRL                = 0x0F,

            PWR_CTRL_1              = 0x10,
            PWR_CTRL_2              = 0x11,
            PWR_CTRL_3              = 0x12,
            PWR_CTRL_4              = 0x13,
            PWR_CTRL_5              = 0x14,

            RAM_ADR_1               = 0x20,
            RAM_ADR_2               = 0x21,
            GRAM_WRITE              = 0x22,

            SOFT_RESET              = 0x28,

            GATE_SCAN               = 0x30,
            VERTICAL_SCROLL_1       = 0x31,
            VERTICAL_SCROLL_2       = 0x32,
            VERTICAL_SCROLL_3       = 0x33,
            PARTIAL_DRIVING_POS_1   = 0x34,
            PARTIAL_DRIVING_POS_2   = 0x35,
            HORIZONTAL_WIN_ADR_1    = 0x36,
            HORIZONTAL_WIN_ADR_2    = 0x37,
            VERTICAL_WIN_ADR_1      = 0x38,
            VERTICAL_WIN_ADR_2      = 0x39,

            GAMMA_CTRL_1            = 0x50,
            GAMMA_CTRL_2            = 0x51,
            GAMMA_CTRL_3            = 0x52,
            GAMMA_CTRL_4            = 0x53,
            GAMMA_CTRL_5            = 0x54,
            GAMMA_CTRL_6            = 0x55,
            GAMMA_CTRL_7            = 0x56,
            GAMMA_CTRL_8            = 0x57,
            GAMMA_CTRL_9            = 0x58,
            GAMMA_CTRL_10           = 0x59,
        };
};

#endif
