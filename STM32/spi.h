/**
    \file spi.h
    \author Arnaud CADOT
    \version 1.0
**/
#ifndef GUARD_SPI
#define GUARD_SPI

#include "hal.h"

#if defined(HAL_SPI_MODULE_ENABLED)

extern "C" void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi);

/**
\brief Basic overlay class for STM32 SPI peripherals
**/
class SPI
{
    public:
        struct Mode
        {
        enum Value
        {
            CPOL_0 = 0x00,
            CPOL_1 = (1 << 1),
            CPHA_0 = 0x00,
            CPHA_1 = (1 << 2),

            MODE_0 = CPOL_0 | CPHA_0,
            MODE_1 = CPOL_0 | CPHA_1,
            MODE_2 = CPOL_1 | CPHA_0,
            MODE_3 = CPOL_1 | CPHA_1,

            USE_PREVIOUS = (1 << 3)
        };
        };

        static constexpr uint32_t READ_TIMEOUT  = 5000L;
        static constexpr uint32_t WRITE_TIMEOUT = 5000L;

        static constexpr uint32_t DEFAULT_FREQUENCY = 0;

        static constexpr Mode::Value DEFAULT_MODE = Mode::MODE_0;

	public:
        /**
        \brief Constructor
        \param spi SPI peripheral
        **/
		SPI(SPI_TypeDef* spi);
	
        /**
        \brief Initialize SPI peripheral
        \returns \c true on error, \c false otherwise
        **/
		bool init(uint32_t frequency = DEFAULT_FREQUENCY, Mode::Value mode = DEFAULT_MODE);
	
        /**
        \brief Write data to SPI peripheral
        \param data Data to be written
        \param dataSize Size of the data
        \returns \c true on error, \c false otherwise
        **/
		bool write(const uint8_t* data, uint16_t dataSize, uint32_t frequency = 0, Mode::Value mode = Mode::USE_PREVIOUS);
        
        /**
        \brief Write a single byte to SPI peripheral
        \param c Byte to be written
        \returns \c true on error, \c false otherwise
        **/
		bool write(uint8_t c, uint32_t frequency = 0, Mode::Value mode = Mode::USE_PREVIOUS);

        /**
        \brief Read data from SPI peripheral
        \param data Destination buffer
        \param dataSize Data to be read
        \returns \c true on error, \c false otherwise
        **/
        bool read(uint8_t* data, uint16_t dataSize, uint32_t frequency = 0, Mode::Value mode = Mode::USE_PREVIOUS);

         /**
        \brief Read and write simultaneously data to/from SPI peripheral
        \param dataOut Outbound data
        \param dataIn Destination buffer
        \param dataSize Data to be written/read
        \returns \c true on error, \c false otherwise
        **/
        bool readWrite(const uint8_t* const dataOut, uint8_t* dataIn, uint16_t dataSize, uint32_t frequency = 0, Mode::Value mode = Mode::USE_PREVIOUS);

        bool isInitialized() const;	

        uint32_t getFloorPrescaler(uint32_t frequency) const;

	private:
		SPI_HandleTypeDef m_handle;

        inline void beginTransaction(uint32_t frequency, Mode::Value mode);
        inline void endTransaction();

        uint32_t m_backupCR1;
	
		void initPins();

};

#if defined(SPI1)
extern SPI spi1;
#endif
#if defined(SPI2)
extern SPI spi2;
#endif

#endif /* #if defined(HAL_SPI_MODULE_ENABLED) */
#endif /* #ifndef GUARD_SPI */
