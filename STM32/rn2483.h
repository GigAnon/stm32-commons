/**
    \file rn2483.h
    \author Arnaud CADOT
    \version 1.0
**/
#ifndef GUARD_RN2483
#define GUARD_RN2483

#include "uart.h"
#include "byte_array.h"
#include "pin.h"

/**
    \brief Class to manage a RN2483 LoRaWan modem over UART

    \remark Use UART 8N1 57000bauds (autobauding features not implemented)
**/
class RN2483
{
    /**
        \brief RN2483 errors
    **/
    struct Error
	{
		/**
			Values
		**/
		enum Type
		{
			NO_ERROR = 0x00,
			INVALID_PARAM,
			KEYS_NOT_INIT,
			NOT_JOINED,
			NO_FREE_CH,
			SILENT,
			FRAME_COUNTER_ERR,
			BUSY,
			MAC_PAUSED,
			INVALID_DATA_LENGTH,
			MAC_ERR,
			UNKNOWN_ERROR
		};
				
		/**
			\brief Convert a ASCII-encoded string into the appropriate Error::Type
		**/
		static Type fromMessage(const ByteArray& s);
	};

    public:
        /**
            \brief Constructor.
            \param serial UART instance used by the peripheral
            \param resetPin Optional reset pin. Set to Pin() to ignore.
        **/
        RN2483(Serial& serial, Pin resetPin = Pin());

        /**
            \brief Reset the peripheral.

            Reset the peripheral. If no reset pin were specified, does nothing.
        **/
        void reset();

        /**
            \brief Join the LoRaWan network
            \returns \c true on error, \c false otherwise.

            Join a LoRaWan network, using OTAA.
        **/
        bool join(ByteArray appEuid, ByteArray appKey);

        /**
            \brief Update method
            \remark This method is non-blocking should be called as often as possible
        **/
        void update();

        /**
            \brief Poll the RN2483 and returns its hardware EUID
            \returns RN2483 EUID in binary format, or ByteArray() on error
        **/
        ByteArray getHardwareEUID();

        /**
            \returns \c true if device is 'connected', \c false otherwise
        **/
        bool isConnected() const;

        /**
            \returns \c true if instance has a downlink payload pending for processing, \c false otherwise
        **/
        bool hasPendingDownlink() const;

        /**
            \brief Get and clear the pending downlink payload
            \returns The pending downlink payload if available, ByteArray() otherwise
        **/
        ByteArray pullPendingDownlink();

        /**
            \brief Send a LoRaWan packet.
            \param payload Packet payload
            \param confirmed Set to \c true to ask for acknowledgement, \c false otherwise.
            \returns \c true on error, \c false otherwise
        **/
        bool send(const ByteArray& payload, bool confirmed = false);

    private:
        Serial& m_serial;
        Pin m_resetPin;

        ByteArray m_rxBuffer;

        ByteArray m_pendingDownlink;

        bool m_connected = false;

        RN2483::Error::Type waitForOK();
};

#endif
