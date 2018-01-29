#ifndef GUARD_LIS3DSH
#define GUARD_LIS3DSH

#include "spi.h"
#include "pin.h"
#include "byte_array.h"

/**
    \brief LIS3DSH driver (4-wire SPI)
**/
class LIS3DSH
{
	public:
		static constexpr uint32_t SPI_FREQUENCY = 10'000'000; ///< Maximum SPI frequency supported by the device

		static constexpr uint8_t PRODUCT_ID		= 0x3F; ///< Product ID (WHO_AM_I register)

        /**
            \brief MEMS data structure
        **/
        struct Data
        {
            /**
                Default constructor
            **/
            Data(): Data(0, 0, 0) {}

            /**
                Constructor
            **/
            Data(int16_t _x, int16_t _y, int16_t _z): x(_x), y(_y), z(_z) {}

            int16_t x; ///< X axis acceleration, in mG
            int16_t y; ///< Y axis acceleration, in mG
            int16_t z; ///< Z axis acceleration, in mG
        };

        /**
            \brief Configuration structure
        **/
        struct Config
        {
            /**
                Refresh data rate
            **/
            enum DataRate
            {
                ODR_POWER_DOWN  = (0b0000),
                ODR_3           = (0b0001),
                ODR_6           = (0b0010),
                ODR_12          = (0b0011),
                ODR_25          = (0b0100),
                ODR_50          = (0b0101),
                ODR_100         = (0b0110),
                ODR_400         = (0b0111),
                ODR_800         = (0b1000),
                ODR_1600        = (0b1001),
            };

            /**
                Constructor
            **/
            Config(DataRate _odr = ODR_50, bool _bdu = false, bool _zen = true, bool _yen = true, bool _xen = true):
                odr(_odr), blockUpdate(_bdu), enableZ(_zen), enableY(_yen), enableX(_xen) {}

            DataRate odr;       //< Refresh data rate
            bool blockUpdate;   ///< BDU feature
            bool enableZ;       ///< Enable Z axis
            bool enableY;       ///< Enable Y axis
            bool enableX;       ///< Enable X axis
        };

	public:
        /**
            \brief Constructor
            \param spi SPI instance to use
            \param cs Chip select pin
        **/
		LIS3DSH(SPI& spi, Pin cs = Pin());

        /**
            \brief Initialize the driver
            \param config Configuration to be used, Config() to use default configuration
            \returns \c true on error, \c false otherwise
        **/
		bool init(Config config = Config());

        /**
            \brief Read X axis
            \returns Value
        **/
		uint16_t readX();

        /**
            \brief Read Y axis
            \returns Value
        **/
		uint16_t readY();

        /**
            \brief Read Z axis
            \returns Value
        **/
		uint16_t readZ();

        /**
            \brief Read temperature
            \returns Value (Celsius)
        **/
        int16_t readTemperature();

        /**
            \brief Read MEMS data
            \returns Value
        **/
        LIS3DSH::Data readData();

        
        /**
            \brief Block until new MEMS data is available
            \returns Value
        **/
        LIS3DSH::Data waitForData();

        /**
            \returns \c true if new MEMS data is available, \c false otherwise
        **/
        bool isDataAvailable();

	private:

		bool write(uint8_t reg, const uint8_t * const data, uint16_t size);
		bool write(uint8_t reg, uint8_t val);

		ByteArray read(uint8_t reg, uint16_t size);
		uint8_t read(uint8_t reg);

		uint16_t read16(uint8_t reg);

		SPI&	m_spi;
		Pin		m_cs;

		struct Registers
		{
			enum Value
			{
				OUT_T				= 0x0C,
				INFO_1				= 0x0D,
			 	INFO_2				= 0x0E,
				WHO_AM_I			= 0x0F,

				OFF_X				= 0x10,
				OFF_Y				= 0x11,
				OFF_Z				= 0x12,

                CS_X				= 0x13,
                CS_Y				= 0x14,
                CS_Z				= 0x15,

                LC_L				= 0x16,
                LC_H				= 0x17,
                LC                  = LC_L,

                STAT                = 0x18,

                PEAK_1              = 0x19,
                PEAK_2              = 0x1A,

                VFC_1               = 0x1B,
                VFC_2               = 0x1C,
                VFC_3               = 0x1D,
                VFC_4               = 0x1E,

                THRS3               = 0x1F,

				CTRL_4				= 0x20,
				CTRL_1				= 0x21,
				CTRL_2				= 0x22,
                CTRL_3	            = 0x23,
                CTRL_5	            = 0x24,
                CTRL_6	            = 0x25,

			    STATUS				= 0x27,

                OUT_X_L             = 0x28,
                OUT_X_H             = 0x29,
			    OUT_X				= OUT_X_L,

                OUT_Y_L             = 0x2A,
                OUT_Y_H             = 0x2B,
                OUT_Y				= OUT_Y_L,

                OUT_Z_L             = 0x2C,
                OUT_Z_H             = 0x2D,
                OUT_Z				= OUT_Z_L,

			    FIFO_CTRL			= 0x2E,
                FIFO_SRC			= 0x2F,
			};
		};
};

#endif
