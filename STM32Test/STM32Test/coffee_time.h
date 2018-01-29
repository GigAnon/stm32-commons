#ifndef GUARD_COFFEE_TIME
#define GUARD_COFFEE_TIME

#include "drivers/ssd1306/ssd1306.h"

/**
    \brief Class managing CoffeeTime's OLED display

    \remark Uses a SSD1306 128x64 OLED display
**/
class CoffeeTime
{
    static constexpr uint32_t FPS = 25;                 ///< Frame rate
    static constexpr uint32_t CHAR_SHIFT_DELAY = 180;   ///< Delay for crawler shift

    static constexpr const char* SPLASHSCREEN_TEXT  = "ACTIMAGE GmbH";  ///< Text displayed in splashscreen
    static constexpr const char* CRAWLER_TEXT = \
    "This service was brought to you by Actimage Kehl        ";     ///< Test displayed in crawler
    static constexpr uint32_t SPLASHSCREEN_DELAY        = 3000;     ///< Duration of full splashscreen display
    static constexpr uint32_t SPLASHSCREEN_FRAME_DELAY  = 50;       ///< Delay between two splashscreen frames

    static constexpr uint16_t MAINS_VOLTAGE = 230;      ///< Mains voltage (for power calculation)
    static constexpr uint16_t MAX_CURRENT   = 30;       ///< Maximum current for amp clamp
    static constexpr uint16_t LOOPS         = 3;        ///< Number of loops in amp clamp
    static constexpr float CALIB_FACTOR     = 1.1214;   ///< Calibration factor
    static constexpr float ANALOG_REF       = 3.3;      ///< Analog reference for ADC (often VDD)

    static constexpr uint32_t REFRESH_DELAY = 1000/FPS;
        
    public:
        /**
            \brief Constructor
            \param screen SSD1306 instance to be used
        **/
        CoffeeTime(SSD1306& screen);

        CoffeeTime(CoffeeTime&)  = delete;
        CoffeeTime(CoffeeTime&&) = delete;

        /**
            \brief Initialize screen & display
        **/
        void init();

        /**
            \brief Update display
            \remark This method should be called as often as possible and is non-blocking
        **/
        void update();

        /**
            \brief Display splashscreen
            \remark This method is blocking
        **/
        void splashscreen();

        /**
            \brief Set value for power meter
            \param value Raw value (i.e. out of ADC) for power meter
        **/
        void setValue(uint16_t value);

        /**
            \brief Set 'connected' indicator
            \param connected \c true to show the 'connected' indicator, \c false for 'disconnected'
        **/
        void setConnected(bool connected);

    private:
        SSD1306& m_screen;

        uint16_t m_value = 0;

        bool m_connected = false;

        int16_t m_bally = 0;
        int16_t m_ballSpeed = 0;

        uint32_t m_lastRefresh = 0;
        uint32_t m_lastCharShift = 0;

        uint16_t m_charIdx = 0;
};

#endif
