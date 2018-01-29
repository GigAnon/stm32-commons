#include "rn2483.h"

#include "system.h"

RN2483::RN2483(Serial& uart, Pin resetPin): m_serial(uart), m_resetPin(resetPin)
{
    m_rxBuffer.reserve(32);

    m_resetPin.init(GPIO_MODE_OUTPUT_PP);
    m_resetPin.setHigh();
}

void RN2483::reset()
{
    m_rxBuffer.clear();
    m_rxBuffer.reserve(32);

    m_resetPin.setLow();
    System::delay(300);
    m_resetPin.setHigh();
    System::delay(100);
    
    ByteArray r;
	do
	{
        m_serial.clear();
		m_serial.write("sys get ver\r\n");
		r = m_serial.readUntil('\n');
	}
	while (!r.startsWith("RN2483"));

}

bool RN2483::join(ByteArray appEuid, ByteArray appKey)
{
    m_connected = false;
 
    m_serial.clear();

    m_serial.write("mac reset 868\r\n");
    //if(waitForOK()) return true;
    waitForOK();

    ByteArray a("mac set deveui ");
    a += getHardwareEUID().asHex();
    a += ByteArray("\r\n");

    m_serial.write(a);
    if(waitForOK()) return true;

    a.clear();
    a += ByteArray("mac set appeui ");
    a += appEuid.asHex();
    a += ByteArray("\r\n");

    m_serial.write(a); 
    if(waitForOK()) return true;

    a.clear();
    a += ByteArray("mac set appkey ");
    a += appKey.asHex();
    a += ByteArray("\r\n");

    m_serial.write(a); 
    if(waitForOK()) return true;

    m_serial.write("mac set adr on\r\n");
    if(waitForOK()) return true;

    char buf[50] = {0};
    for(int i=0;i<3;++i)
	{
        snprintf(buf, sizeof(buf), "mac set ch dcycle %d 0\r\n", i);
		m_serial.write(buf);
		if (waitForOK()) return true;
	}
    

    m_serial.write("mac save\r\n");
    if(waitForOK()) return true;

    m_serial.write("mac join otaa\r\n");
    if(waitForOK()) return true;

    return false;
}

void RN2483::update()
{
    m_rxBuffer.reserve(32);

	if (m_serial.dataAvailable())
		 m_serial.readAppend(m_rxBuffer);

	while (true)
	{
		unsigned int rxAv = m_rxBuffer.size();

		if (rxAv < 2)
			return;

		unsigned int idx = 0;

		while (idx < rxAv && m_rxBuffer[idx] != '\n')
			++idx;

		if (idx == rxAv)
			return;
		
		ByteArray msg(m_rxBuffer.reverseSplit(idx+1));

		if(msg.startsWith("accepted"))
		{
			printf("LoRa OK\r\n");
            m_connected = true;
		}
		else if(msg.startsWith("denied"))
		{
			printf("Cxn failed\r\n");
            m_connected = false;
		}
		else if (msg.startsWith("mac_tx_ok"))
		{
			printf("Tx OK\r\n");
		}
		else if (msg.startsWith("mac_err"))
		{
			printf("Tx error\r\n");
            m_connected = true;
		}
		else if (msg.startsWith("mac_rx 1 "))
		{
            printf("Confirmed Tx OK\r\n");
            m_connected = true;

            if (msg.size() > 11)
            {
                ByteArray payload = msg.split(9);
                printf("Downlink data: ");
                PRINTF_UART.write(payload);
                m_pendingDownlink = ByteArray::fromHex(payload);
            }

		}
	}
}

ByteArray RN2483::getHardwareEUID()
{
	m_serial.write("sys get hweui\r\n");
	ByteArray addr = m_serial.readUntil('\n');

	return ByteArray::fromHex(addr);
}


bool RN2483::isConnected() const
{
    return m_connected;
}

bool RN2483::hasPendingDownlink() const
{
    return m_pendingDownlink.size() > 0;
}

ByteArray RN2483::pullPendingDownlink()
{
    ByteArray a;
    a.swap(m_pendingDownlink);
    return a;
}

RN2483::Error::Type RN2483::waitForOK()
{
	ByteArray m(m_serial.readUntil('\n')); 

	Error::Type e = Error::fromMessage(m);
	
	if(e != Error::NO_ERROR)
	{
		printf("LoRaWan error: ");
		PRINTF_UART.write(m);
	}

	return e;
}


RN2483::Error::Type RN2483::Error::fromMessage(const ByteArray& s)
{
	if (s.startsWith("ok"))
		return Error::NO_ERROR;
	if (s.startsWith("invalid_param"))
		return Error::INVALID_PARAM;
	if (s.startsWith("keys_not_init"))
		return Error::KEYS_NOT_INIT;
	if (s.startsWith("no_free_ch"))
		return Error::NO_FREE_CH;
	if (s.startsWith("silent"))
		return Error::SILENT;
	if (s.startsWith("busy"))
		return Error::BUSY;
	if (s.startsWith("mac_paused"))
		return Error::MAC_PAUSED;
	if (s.startsWith("invalid_data_len"))
		return Error::INVALID_DATA_LENGTH;
	if (s.startsWith("mac_err"))
		return Error::MAC_ERR;

	return Error::UNKNOWN_ERROR;
}

bool RN2483::send(const ByteArray& payload, bool confirmed)
{
    ByteArray s("mac tx ");
    s += (confirmed ? ByteArray("cnf") : ByteArray("uncnf"));
    s += ByteArray(" 1 ");
	s += payload.asHex();
    s.append('\r');
    s.append('\n');

    m_serial.write(s);

    return waitForOK();
}
