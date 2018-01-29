#ifndef GUARD_SSD2119
#define GUARD_SSD2119

#include "hal.h"
#include "pin.h"
#include "display.h"
#include "filesystem/file.h"

/* HAL (possible?) patch: in stm32f4xx_ll_fsmmc.c:FSMC_NORSRAM_Init(), 
Replace:
tmpr = Device->BTCR[Init->NSBank];
By:
tmpr = 0;
*/

/**
    \brief Driver for SSD2119 LCD display

    Driver for SSD2119 LCD display
**/
class SSD2119: public Display
{
        volatile uint16_t* const cmdAdr =  ((volatile uint16_t* const)(0x60000000|0x00000000));
        volatile uint16_t* const baseAdr = ((volatile uint16_t* const)(0x60000000|0x00100000));

        static constexpr uint32_t LCD_WIDTH     = 320; /* x axis */
        static constexpr uint32_t LCD_HEIGHT    = 240; /* y axis */

    public:
        SSD2119(Pin rst = Pin(), Pin backlight = Pin());

        virtual bool init(Orientation orientation = Orientation::PORTRAIT_1);
        virtual void writePixel(int16_t x, int16_t y, uint16_t color);

        virtual inline void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { writeFillRect(x, y, w, h, color); }
        
        bool drawBmpFromFile(File& file, int32_t x, int32_t y, bool invert = true);

        virtual void setOrientation(Orientation orientation);

    private:
        
        bool initFSMC();
        bool initGpio();

        void reset();

        virtual void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

        inline void __attribute__((optimize("O0"))) writeCmd(volatile uint16_t cmd) { *cmdAdr = cmd; };
        inline void __attribute__((optimize("O0"))) writeReg(volatile uint16_t reg, volatile uint16_t val) { *cmdAdr = reg; *baseAdr = val; }
        inline void __attribute__((optimize("O0"))) writeRam(volatile uint16_t c) { *baseAdr = c; }

        uint16_t readReg(uint16_t reg);

        SRAM_HandleTypeDef m_ram = {};
        Pin m_rst;
        Pin m_backlight;

        enum Registers
        {
            DEVICE_CODE_READ  = 0x00,
            OSC_START         = 0x00,
            OUTPUT_CTRL       = 0x01,
            LCD_DRIVE_AC_CTRL = 0x02,
            PWR_CTRL_1        = 0x03,

            DISPLAY_CTRL      = 0x07,

            FRAME_CYCLE_CTRL  = 0x0B,
            PWR_CTRL_2        = 0x0C,
            PWR_CTRL_3        = 0x0D,
            PWR_CTRL_4        = 0x0E,
            GATE_SCAN_START   = 0x0F,
            SLEEP_MODE_1      = 0x10,
            ENTRY_MODE        = 0x11,
            SLEEP_MODE_2      = 0x12,

            GEN_IF_CTRL       = 0x15,

            PWR_CTRL_5        = 0x1E,

            RAM_DATA          = 0x22,

            FRAME_FREQ        = 0x25,
            ANALOG_SET        = 0x26,

            VCOM_OTP_1        = 0x28,
            VCOM_OTP_2        = 0x29,
            GAMMA_CTRL_1      = 0x30,
            GAMMA_CTRL_2      = 0x31,
            GAMMA_CTRL_3      = 0x32,
            GAMMA_CTRL_4      = 0x33,
            GAMMA_CTRL_5      = 0x34,
            GAMMA_CTRL_6      = 0x35,
            GAMMA_CTRL_7      = 0x36,
            GAMMA_CTRL_8      = 0x37,

            GAMMA_CTRL_9      = 0x3A,
            GAMMA_CTRL_10     = 0x3B,

            V_RAM_POS         = 0x44,
            H_RAM_START       = 0x45,
            H_RAM_END         = 0x46,

            X_RAM_ADDR        = 0x4E,
            Y_RAM_ADDR        = 0x4F,
        };
};

#endif
