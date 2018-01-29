#ifndef GUARD_AXSIGFOX
#define GUARD_AXSIGFOX

#include "uart.h"
#include "byte_array.h"

/**
    \brief Basic class for communicating with AX-SFEU Sigfox modem
**/
class AXSigfox
{
        constexpr static uint16_t MAX_MSG_SIZE = 64;                ///< Maximum size of an inbound message
        constexpr static uint32_t DEFAULT_DEVICE_TIMEOUT = 2000;    ///< Timeout delay for device to answer
        constexpr static uint32_t NETWORK_TIMEOUT = 60000;          ///< Timeout delay before an outbound message is considered lost

    public:
        /**
            \brief Constructor
            \param serial UART the modem is connected to
        **/
        AXSigfox(Serial& serial);

        AXSigfox(const AXSigfox&) = delete;
    
        /**
            \brief Update method
            \remark This method must be called regularly
            \returns \c true if downlink data was received, \c false otherwise
        **/
        bool update();

        /**
            \brief 'ping' the device
            \returns \c true if the modem was detected, \c false otherwise
        **/
        bool isDeviceConnected();

        /**
            \brief Send a Sigfox frame
            \param payload Payload to send
            \param ack Set to \c true to ask for acknowledgement, \c false otherwise
            \returns \c true on error, \c false otherwise

            \remark Because of the way the Sigfox protocol was designed, to get downlink messages
            the 'ack' field must be set to \c true. Be aware this *will* count against your daily
            *downlink* quota.
        **/
        bool sendFrame(const ByteArray& payload, bool ack=false);

        ByteArray getLatestRxData() const;
    
    private:
        Serial& m_serial;
    
        uint8_t m_buffer[MAX_MSG_SIZE] = {0};
        uint16_t m_idx = 0;

        ByteArray m_rxData;

        bool m_messagePending = false;
        uint32_t m_messageTimestamp = 0;

        bool processMessage(uint8_t* d, uint16_t s);
        
        void write(const char* str);
        bool waitForOk(uint32_t timeout = DEFAULT_DEVICE_TIMEOUT);
};


#endif