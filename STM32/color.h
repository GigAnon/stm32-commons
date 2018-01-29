#ifndef GUARD_COLOR
#define GUARD_COLOR

#include <cstdint>

/**
    16-bits, 5-6-5 color class
**/
class Color16
{
    public:
        /**
            \brief Constructor
            \param colorRgb uint16_t color
        **/
        Color16(uint16_t colorRgb = 0);

        /**
            \brief Constructor
            \param r Red component
            \param g Green component
            \param b Blue component
        **/
        Color16(uint8_t r, uint8_t g, uint8_t b);

        /**
            \returns uint16_t representation of the color
        **/
        uint16_t toUInt16() const;

        /**
            \returns uint16_t representation of the color
        **/
        operator uint16_t() const { return toUInt16(); }

        /**
            \returns Red component
        **/
        uint8_t red() const;

        /**
            \returns Green component
        **/
        uint8_t green() const;

        /**
            \returns Blue component
        **/
        uint8_t blue() const;

        /**
            \brief Convert R, G, B components to uint16_t 5-6-5 representation
            \param r Red component
            \param g Green component
            \param b Blue component
            \returns uint16_t representation of the color
        **/
        static uint16_t toUInt16Rgb(uint8_t r, uint8_t g, uint8_t b);

    private:

        uint16_t m_color;

    public:
        const static Color16 Black;
        const static Color16 White;
        const static Color16 Red;
        const static Color16 Green;
        const static Color16 Blue;
};


#endif