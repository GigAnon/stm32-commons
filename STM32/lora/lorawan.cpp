#include "lorawan.h"

#include <algorithm>
#include "system.h"

extern "C"
{
    #include "mac/loramac.h"
    #include "mac/loramac_test.h"
    #include "utilities/low_power.h"
}

LoRaWan lorawan;

constexpr size_t LoRaWan::TX_BUFFER_SIZE;

uint8_t HW_GetBatteryLevel()
{
    return 0xFF;
}

LoRaWan::LoRaWan()
{}

bool LoRaWan::init(SPI* spi, DataRateConfig::Value drConfig)
{
    if(!spi)
        return true;

    m_spi = spi;

    Radio.IoInit();
    HW_RTC_Init();

    m_state = DeviceState::INIT;

    printf("OTAA\n\r"); 
    printf("DevEui= %02X", m_devEuid[0]); for(int i=1; i<8 ; ++i) {printf("-%02X", m_devEuid[i]); }; printf("\n\r");
    printf("AppEui= %02X", m_appEuid[0]); for(int i=1; i<8 ; ++i) {printf("-%02X", m_appEuid[i]); }; printf("\n\r");
    printf("AppKey= %02X", m_appKey[0]);  for(int i=1; i<16; ++i) {printf(" %02X", m_appKey[i]); }; printf("\n\n\r");

    return false;
}

bool LoRaWan::setComissioning(	const ByteArray& devEuid,
								const ByteArray& appEuid,
								const ByteArray& appKey)
{
	if(devEuid.size() != 8 || appEuid.size() != 8 || appKey.size() != 16)
		return true;

    memcpy(m_devEuid, devEuid.internalBuffer(), 8);
	memcpy(m_appEuid, appEuid.internalBuffer(), 8);
	memcpy(m_appKey,  appKey.internalBuffer(),  16);

    return false;
}

bool LoRaWan::send(const ByteArray& data, bool confirmed)
{
    if(data.size() >= 64)
        return true;

    if(!isNetworkJoined())
        return true;

    if(m_state != DeviceState::SLEEP)
        return true;

    m_pendingFrame.confirmed    = confirmed;
    m_pendingFrame.data         = data;

    m_state     = DeviceState::SEND;
    m_nextTx    = true;

    return false;
}

bool LoRaWan::hasPendingRx() const
{
    return m_pendingRx.size();
}

ByteArray LoRaWan::pullPendingRx()
{
    ByteArray t;
    t.swap(m_pendingRx);
    return t;
}

bool LoRaWan::sendFrame()
{
    if(!m_pendingFrame)
        return false;

    const uint8_t port = 1;
    int8_t dr = m_drConfig == DataRateConfig::ADR ? DR_0 : m_drConfig;

    size_t s = std::min(TX_BUFFER_SIZE, m_pendingFrame.data.size());

    memcpy(m_txBuffer, m_pendingFrame.data.internalBuffer(), s);

    m_pendingFrame = Frame();

    McpsReq_t mcpsReq;
    LoRaMacTxInfo_t txInfo;
    
    if(LoRaMacQueryTxPossible(s, &txInfo) != LORAMAC_STATUS_OK)
    {
        // Send empty frame in order to flush MAC commands
        mcpsReq.Type                        = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fBuffer     = nullptr;
        mcpsReq.Req.Unconfirmed.fBufferSize = 0;
        mcpsReq.Req.Unconfirmed.Datarate    = dr;
    }
    else
    {
        if(m_pendingFrame.confirmed)
        {
            mcpsReq.Type = MCPS_CONFIRMED;
            mcpsReq.Req.Confirmed.fPort         = port;
            mcpsReq.Req.Confirmed.fBuffer       = m_txBuffer;
            mcpsReq.Req.Confirmed.fBufferSize   = s;
            mcpsReq.Req.Confirmed.NbTrials      = 8;
            mcpsReq.Req.Confirmed.Datarate      = dr;
        }
        else
        {
            mcpsReq.Type = MCPS_UNCONFIRMED;
            mcpsReq.Req.Unconfirmed.fPort       = port;
            mcpsReq.Req.Unconfirmed.fBuffer     = m_txBuffer;
            mcpsReq.Req.Unconfirmed.fBufferSize = s;
            mcpsReq.Req.Unconfirmed.Datarate    = dr;
        }
    }

    return LoRaMacMcpsRequest(&mcpsReq) != LORAMAC_STATUS_OK;
}

void LoRaWan::McpsConfirm(McpsConfirm_t *mcpsConfirm)
{
    lorawan.mcpsConfirm(mcpsConfirm);
}

void LoRaWan::mcpsConfirm(McpsConfirm_t *mcpsConfirm)
{
    m_nextTx = true;
}

void LoRaWan::McpsIndication(McpsIndication_t *mcpsIndication)
{
    lorawan.mcpsIndication(mcpsIndication);
}

void LoRaWan::mcpsIndication(McpsIndication_t *mcpsIndication)
{
    if( mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
        return;
    }

    if(mcpsIndication->RxData)
    {
        uint8_t port = mcpsIndication->Port;

        if(mcpsIndication->BufferSize)
            lorawan.m_pendingRx = ByteArray(mcpsIndication->Buffer, mcpsIndication->BufferSize);
    }
}

void LoRaWan::MlmeConfirm(MlmeConfirm_t *mlmeConfirm)
{
    lorawan.mlmeConfirm(mlmeConfirm);
}

void LoRaWan::mlmeConfirm(MlmeConfirm_t *mlmeConfirm)
{
    switch(mlmeConfirm->MlmeRequest )
    {
        case MLME_JOIN:
        {
            if(mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK)
                m_state = DeviceState::JOINED;
            else
                m_state = DeviceState::JOIN;

            break;
        }
        case MLME_LINK_CHECK:
        {
            if(mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK)
            {
                // Check DemodMargin
                // Check NbGateways
                m_latestLinkcheckData.gatewayCount  = mlmeConfirm->NbGateways;
                m_latestLinkcheckData.margin        = mlmeConfirm->DemodMargin;
            }
            break;
        }
        default:
            break;
    }
    m_nextTx = true;
}


void LoRaWan::update()
{
    switch(m_state)
    {
        case DeviceState::INIT:
        {
            m_loramacPrimitives.MacMcpsConfirm    = McpsConfirm;
            m_loramacPrimitives.MacMcpsIndication = McpsIndication;
            m_loramacPrimitives.MacMlmeConfirm    = MlmeConfirm;

            m_loramacCallbacks.GetBatteryLevel    = HW_GetBatteryLevel;
            m_nextTx = true;
            m_latestLinkcheckData = LinkcheckStats();

            #if defined(REGION_EU868)
                LoRaMacInitialization(&m_loramacPrimitives, &m_loramacCallbacks, LORAMAC_REGION_EU868);
            #else
                #error "Please define a region in the compiler options."
            #endif

            MibRequestConfirm_t mibReq;

            mibReq.Type             = MIB_ADR;
            mibReq.Param.AdrEnable  = (m_drConfig == DataRateConfig::ADR);
            LoRaMacMibSetRequestConfirm( &mibReq );

            mibReq.Type                         = MIB_PUBLIC_NETWORK;
            mibReq.Param.EnablePublicNetwork    = true;
            LoRaMacMibSetRequestConfirm(&mibReq);
                        
            mibReq.Type         = MIB_DEVICE_CLASS;
            mibReq.Param.Class  = CLASS_A;
            LoRaMacMibSetRequestConfirm(&mibReq);

            #if defined(REGION_EU868)
                LoRaMacTestSetDutyCycleOn(false);
            #endif

            m_state = DeviceState::JOIN;
        }
        break;
        case DeviceState::JOIN:
        {
            MlmeReq_t mlmeReq;
    
            mlmeReq.Type = MLME_JOIN;
            mlmeReq.Req.Join.DevEui = m_devEuid;
            mlmeReq.Req.Join.AppEui = m_appEuid;
            mlmeReq.Req.Join.AppKey = m_appKey;
            mlmeReq.Req.Join.NbTrials = JOIN_RETRIES;

            if(m_nextTx)
                LoRaMacMlmeRequest(&mlmeReq);

            m_state = DeviceState::SLEEP;
        }
        break;
        case DeviceState::JOINED:
        {
            printf("JOINED\n\r");

            sendLinkCheck();

            m_state = DeviceState::SEND;
            break;
        }
        case DeviceState::SEND:
        {
            if(m_nextTx)
                m_nextTx = sendFrame();

            m_state = DeviceState::SLEEP;
            break;
        }
        case DeviceState::SLEEP:
            break; // Wake up through events
        default:
            m_state = DeviceState::INIT;
            break;
    }
}

bool LoRaWan::sendLinkCheck()
{
    if(!isNetworkJoined() || !m_nextTx)
        return true;

    MlmeReq_t mlmeReq;
    mlmeReq.Type = MLME_LINK_CHECK;

    return LoRaMacMlmeRequest(&mlmeReq) != LORAMAC_STATUS_OK;
}

LoRaWan::LinkcheckStats LoRaWan::getLatestLinkcheck() const
{
    return m_latestLinkcheckData;
}

bool LoRaWan::hasValidLinkcheck() const
{
    return m_latestLinkcheckData.isValid();
}

bool LoRaWan::isNetworkJoined() const
{
    MibRequestConfirm_t mib;

    mib.Type = MIB_NETWORK_JOINED;

    LoRaMacMibGetRequestConfirm(&mib);

    return mib.Param.IsNetworkJoined;
}

bool LoRaWan::isStopAllowed() const
{
    return LowPower_GetState() == 0;
}

int16_t LoRaWan::getRSSI() const
{
    return Radio.Rssi(MODEM_LORA);
}

bool LoRaWan::setTxPower(uint8_t txPower)
{
    MibRequestConfirm_t request = {};

    request.Type                    = MIB_CHANNELS_TX_POWER;
    request.Param.ChannelsTxPower   = txPower;

    return LORAMAC_STATUS_OK == LoRaMacMibSetRequestConfirm(&request);
}

uint8_t LoRaWan::getTxPower() const
{
    MibRequestConfirm_t request = {};

    request.Type = MIB_CHANNELS_TX_POWER;

    if(LoRaMacMibGetRequestConfirm(&request) != LORAMAC_STATUS_OK)
        return 0xFF;

    return request.Param.ChannelsTxPower;
}


uint16_t HW_SPI_InOut(uint16_t txData)
{
    uint16_t rxData;

    /** Mode switch is broken and disabled ATM **/
    /** @todo Fix it? **/
    lorawan.m_spi->readWrite(reinterpret_cast<uint8_t*>(&txData), reinterpret_cast<uint8_t*>(&rxData), 1, 10000000, SPI::Mode::MODE_0);

    return rxData;
}
