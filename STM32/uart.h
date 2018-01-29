/**
    \file uart.h
    \author Arnaud CADOT
    \version 1.0
**/
#ifndef GUARD_UART
#define GUARD_UART

#include "hal.h"

#if defined(HAL_UART_MODULE_ENABLED)

#include "circular_buffer.h"
#include "byte_array.h"

#if defined(USART1)
extern "C" void USART1_IRQHandler(void);
#endif
#if defined(USART2)
extern "C" void USART2_IRQHandler(void);
#endif
#if defined(USART3)
extern "C" void USART3_IRQHandler(void);
#endif

/**
\brief Basic Serial class for communications over RS232 8N1, no flow control.
RX is based on interruption + a circular buffer
TX is purely polling-based

This class is not designed to be fast, particularly on outbound data. It is
designed to be convenient for most use cases.
**/
class Serial
{
		static constexpr uint16_t RX_BUFFER_SIZE = 32; ///< RX buffer size
	
	public:
        /**
        \brief Constructor.
        \param uart UART peripheral to be used
        **/
		Serial(USART_TypeDef* uart);
	    
        /**
        \brief Initialize UART peripheral
        \param baudrate Baudrate to be used
        \param useAF Set to \c true to use alternate pin mapping, \c false otherwise
        \returns \c true on error, \c false otherwise
        **/
		bool init(uint32_t baudrate, bool useAF = false);
		
        /**
        \brief Clear the RX buffer
        **/
        void clear();

        /**
        \brief Write multiple bytes
        \param data Data to be written
        \param dataSize Data size
        \returns \c true on error, \c false otherwise
        **/
		bool write(const uint8_t* data, size_t dataSize);

        /**
        \brief Write a nul-terminated c-string
        \param str String to be written
        \returns \c true on error, \c false otherwise
        **/
        bool write(const char* str);

        /**
        \brief Write a single byte
        \param c Byte to be written
        \returns \c true on error, \c false otherwise
        **/
        bool write(uint8_t c);

        /**
        \brief Write a ByteArray
        \param ba ByteArray to be written
        \returns \c true on error, \c false otherwise
        **/
        bool write(const ByteArray& ba);
	
        /**
        \brief Read multiple bytes from RX buffer
        \param data Destination buffer
        \param maxBytes Maximum bytes to be read
        \return Bytes read
        **/
		size_t read(uint8_t* data, size_t maxBytes);

        /**
        \brief Read a single byte from RX buffer
        \return Byte read, or 0 on error
        **/
        uint8_t read();

        /**
        \brief Read multiple bytes and append them to an existing ByteArray
        \param a ByteArray to append to
        \param maxBytes Maximum bytes to be read
        **/
        size_t readAppend(ByteArray& a, uint16_t maxBytes = 0xFFFF);

        /**
        \brief Block and read incoming data until \c endc is found or \c maxBytes
        bytes have been read
        \param endc End character
        \param maxBytes Maximum bytes to be read
        \return Bytes read
        **/
        ByteArray readUntil(uint8_t endc, uint16_t maxBytes = 0xFF);
	    
        /**
        \returns Data available in RX buffer
        **/
		size_t dataAvailable() const;
	private:
		UART_HandleTypeDef m_handle;
	
		CircularBuffer m_rxBuffer;
		
		void initPins(bool useAF);
	
		void rxInteruptHandler();

    #if defined(USART1)
		friend void ::USART1_IRQHandler(void);
    #endif
    #if defined(USART2)
		friend void ::USART2_IRQHandler(void);
    #endif
    #if defined(USART3)
		friend void ::USART3_IRQHandler(void);
    #endif
};

#if defined(USART1)
    extern Serial uart1;
#endif

#if defined(USART2)
    extern Serial uart2;
#endif

#if defined(USART3)
    extern Serial uart3;
#endif

#ifndef PRINTF_UART
    #define PRINTF_UART uart2
#endif

#endif /* #if defined(HAL_RNG_MODULE_ENABLED) */
#endif /* #ifndef GUARD_UART */
