#ifndef GUARD_XPT2046
#define GUARD_XPT2046

#include "pin.h"
#include "spi.h"

#include "touch.h"

/**
    \brief Driver for XPT2046 touch screen sensor
**/
class XPT2046: public Touch
{
    public:
        static constexpr uint16_t DEFAULT_SIZE_X        = 320;  ///< Default size on X axis
        static constexpr uint16_t DEFAULT_SIZE_Y        = 240;  ///< Default size on Y axis
        static constexpr uint16_t DEFAULT_Z_THRESHOLD   = 150;  ///< Default threshold for pressure detection

        static constexpr uint16_t RAW_MAX_X             = 4096;
        static constexpr uint16_t RAW_MAX_Y             = 4096;

        static constexpr uint32_t UPDATE_MIN_DELAY      = 1;

        static constexpr uint8_t  NUM_SAMPLES           = 5;    ///< Number of samples for pressure detection

        static constexpr uint32_t SPI_MAX_FREQUENCY     = 4500000;  ///< Maximum SPI frequency supported by the hardware

    public:
        /**
            \brief Constructor
            \param spi SPI interface to be used
            \param cs Optional chip select pin
            \param size Screen size
            \param zThreshold Touch detection pressure threshold

        **/
        XPT2046(SPI& spi, Pin cs = Pin(),
                Point2D size = Point2D(DEFAULT_SIZE_X, DEFAULT_SIZE_Y),
                uint16_t zThreshold = DEFAULT_Z_THRESHOLD);

        /**
            \brief Initialize the driver
            \param orientation Screen orientation
            \returns \c true on error, \c false otherwise
        **/
        virtual bool init(Orientation orientation = Orientation::LANDSCAPE_2);

        /**
            \brief Update method. Must be called at regular intervals
            \returns Pair: (pressed, point), where \c pressed is \c true if the screen is being
            touched, in which case \c point are the coordinates of the press
        **/
        virtual std::pair<bool, Point2D> update();

        /**
            \brief Set the calibration data for the touch screen
            \param minCorner Real upper-left corner position
            \param stretchX Linear stretch factor for X axis
            \param stretchY Linear stretch factor for Y axis
        **/
        void setCalibrationData(Point2D minCorner, float x, float y);

    protected:
        /**
            \brief Read X, Y & Z data from sensor
            \param point (X, Y) data will be written here
            \param z Z data will be written here
            \returns \c true on error, \c false otherwise

            \remark This is raw, unfiltered data from the sensor. (X, Y) data will make
            no sense if nothing is touching the screen.
        **/
        bool readAll(Point2D& point, int16_t& z);

        /**
            \brief Convert 'raw' coordinates into 'real' coordinates, using calibration and orientation data.
            \param raw Raw coordinates
            \returns Real coordinates
        **/
        Point2D rawToReal(Point2D raw) const;

    private:
        SPI&        m_spi;
        Pin         m_cs;

        Point2D     m_size;
        uint16_t    m_zThreshold;

        uint32_t    m_lastUpdate = 0;
        std::pair<bool, Point2D> m_cachedResult;
};

#endif