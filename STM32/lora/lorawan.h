#ifndef GUARD_LORAWAN
#define GUARD_LORAWAN

extern "C"
{
    #include "phy/radio.h"
    #include "wan/hw.h"
    #include "wan/comissioning.h"
    #include "mac/loramac.h"
    #include "mac/region/Region.h"
}

#include "spi.h"
#include "byte_array.h"

/**
    \brief Barebone OO overlay for LoRaMac-Node

    \remark Adapted from ST's own overlay. Refactored for convenience.
**/
class LoRaWan
{
	public:
        /**
            Data rate configuration structure.
            Can be fixed or dynamic (ADR).
        **/
		struct DataRateConfig
		{
            /**
                Value
            **/
			enum Value
			{
				ADR = 0xFF, ///< Use Adaptive Data Rate
				DR0 = DR_0, ///< Use fixed DR0 (highest range, lowest bitrate)
				DR1 = DR_1, ///< Use fixed DR1
				DR2 = DR_2, ///< Use fixed DR2
				DR3 = DR_3, ///< Use fixed DR3
				DR4 = DR_4, ///< Use fixed DR4
				DR5 = DR_5  ///< Use fixed DR5 (lowest range, highest bitrate)
			};
		};

        /**
            FSM states
        **/
        enum class DeviceState
        {
            INIT,
            JOIN,
            JOINED,
            SEND,
            CYCLE,
            SLEEP
        };

        /**
            TX message structure
        **/
        struct Frame
        {
            ByteArray   data;
            bool        confirmed   = false;
            uint8_t     port        = 1;

            bool isValid() const { return data.size() != 0; }
            operator bool() const { return isValid(); }
        };

        /**
            Structure for Link Check command data
        **/
        struct LinkcheckStats
        {
            uint8_t margin       = 0xFF;
            uint8_t gatewayCount = 0;

            bool isValid() const { return margin != 0xFF && gatewayCount != 0; }
            operator bool() const { return isValid(); }
        };

        constexpr static size_t TX_BUFFER_SIZE = 64; ///< Size of the TX buffer
        constexpr static uint8_t JOIN_RETRIES = 3;  ///< Maximum JOIN_REQUEST retries

    public:
        /**
            Constructor.
        **/
        LoRaWan();

        /**
            Initialize radio and mac layers and initiate an OTAA join.
            \param spi SPI peripheral to use
            \param drConfig Data rate configuration
            \returns \c true on error, \c false otherwise
            \remark After changing comissionning parameters, this method must be called again.
        **/
        bool init(SPI* spi, DataRateConfig::Value drConfig = DataRateConfig::ADR);

        /**
            Update method.
            Must be called as often as possible.
        **/
        void update();

        /**
            \brief Send a LoRaWan message
            \param data Message payload
            \param confirmed Set to \c true to ask for ack, \c false otherwise
            \returns \c true on error, \c false otherwise
        **/
        bool send(const ByteArray& data, bool confirmed = false);

        bool sendLinkCheck();

        /**
            \brief Set OTAA commionning data
            \param devEuid Device EUID (8 bytes)
            \param appEuid Application EUID (8 bytes)
            \param appKey Application Key (16 bytes)
            \returns \c true on error, \c false otherwise

            \remark init() should be called after changing those parameters to reconnect to the network
            \todo Make the implementation less of a kludge
        **/
		bool setComissioning(	const ByteArray& devEuid,	/* 8 Bytes */
								const ByteArray& appEuid,	/* 8 Bytes */
								const ByteArray& appKey);	/* 16 Bytes */

        /**
            \brief Set TX power for all channels
            \param txPower TX power index (region dependant)
            \returns \c true on error, \c false otherwise
        **/
        bool setTxPower(uint8_t txPower);

        /**
            \brief Get the current TX power
            \returns TX power index (0xFF on error)
        **/
        uint8_t getTxPower() const;

        /**
            \returns \c true if a downlink message is pending processing, \c false otherwise
        **/
        bool hasPendingRx() const;

        /**
            \brief Get pending downlink message and clear internal cache
            \returns Pending downlink message, or ByteArray() if there was none to pull
        **/
        ByteArray pullPendingRx();

        /**
            \returns \c true if OTAA was successful, \c false otherwise
            \remark It does not mean the device can still access the network, only that the 
            initial authentication was successful.
        **/
        bool isNetworkJoined() const;

        LinkcheckStats getLatestLinkcheck() const;

        bool hasValidLinkcheck() const;

        /**
            \returns RSSI value of the last received packet.
            \todo Make it work!
        **/
        int16_t getRSSI() const;

        bool isStopAllowed() const;

        static void McpsConfirm     (McpsConfirm_t*     mcpsConfirm);
        static void McpsIndication  (McpsIndication_t*  mcpsIndication);
        static void MlmeConfirm     (MlmeConfirm_t*     mlmeConfirm);

    private:

        bool sendFrame();

        friend uint16_t HW_SPI_InOut(uint16_t txData);

        void mcpsConfirm    (McpsConfirm_t *mcpsConfirm);
        void mcpsIndication (McpsIndication_t *mcpsIndication);
        void mlmeConfirm    (MlmeConfirm_t *mlmeConfirm);

        bool m_nextTx;

        Frame m_pendingFrame;
        ByteArray m_pendingRx;

        uint8_t m_txBuffer[TX_BUFFER_SIZE] = {};

        uint8_t m_devEuid[8]  = {};
        uint8_t m_appEuid[8]  = {};
        uint8_t m_appKey [16] = {};

        LinkcheckStats m_latestLinkcheckData;

        DataRateConfig::Value m_drConfig    = DataRateConfig::ADR;

        DeviceState m_state                 = DeviceState::INIT;

        LoRaMacCallback_t   m_loramacCallbacks  = {};
        LoRaMacPrimitives_t m_loramacPrimitives = {};

        SPI* m_spi = nullptr;
};

extern LoRaWan lorawan; ///< Singleton instance

#endif
