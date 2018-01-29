#include "axsigfox.h"

#include "system.h"

#include <cstring>

AXSigfox::AXSigfox(Serial& serial): m_serial(serial)
{}

bool AXSigfox::update()
{
    if(m_messagePending && System::millis() - m_messageTimestamp > NETWORK_TIMEOUT)
    {
        /* Error handling? */
        m_messagePending = false;
    }

    bool r = false;

    while(m_serial.dataAvailable())
    {
        uint8_t c = 0;
        m_serial.read(&c, 1);
        
        if(c == '\n')
        {
            if(m_idx > 1 && m_buffer[m_idx-1] == '\r')
            {
                m_buffer[m_idx-1] = 0;
                r |= processMessage(m_buffer, m_idx-1);
            }
            else
            {
                /* Error handling? */
            }

            m_idx = 0;
            continue;
        }

        m_buffer[m_idx++] = c;

        if(m_idx == MAX_MSG_SIZE)
        {
            /* Error handling ? */
            m_idx = 0;
            continue;
        }
    }

    return r;
}

bool AXSigfox::processMessage(uint8_t* d, uint16_t s)
{
    ByteArray msg(d, s);
    bool r = false;

    if(msg.startsWith("OK"))
    {
        if(m_messagePending)
            m_messagePending = false;
    }
    else if(msg.startsWith("RX="))
    {
        m_rxData = ByteArray::fromHex(msg.split(3));
        r = true;
    }

    printf("%s\r\n", d);
    return r;
}

ByteArray AXSigfox::getLatestRxData() const
{
    return m_rxData;
}

bool AXSigfox::sendFrame(const ByteArray& payload, bool ack)
{
    if(payload.size() < 1 || payload.size() > 12)
        return true;

    if(m_messagePending)
        return true;

    ByteArray p("AT$SF=");
    p += payload.asHex();

    if(ack)
        p += ByteArray(",1");

    p.append('\n');

    m_serial.write(p);

    m_messagePending = true;
    m_messageTimestamp = System::millis();

    return false;
}

bool AXSigfox::isDeviceConnected()
{
    write("AT");
    return !waitForOk(1000);
}

void AXSigfox::write(const char* str)
{
    m_serial.write(str);
    m_serial.write('\r');
    m_serial.write('\n');
}

bool AXSigfox::waitForOk(uint32_t timeout)
{
    uint32_t start = System::millis();

    uint8_t ok[] = "OK\r\n";

    while(m_serial.dataAvailable() < sizeof(ok)-1)
        if(timeout && System::millis() - start > timeout)
            return true;

    uint8_t buff[sizeof(ok)] = {0};

    m_serial.read(buff, sizeof(buff));

    for(size_t i=0;i<sizeof(ok);++i)
        if(buff[i] != ok[i])
            return true;

    return false;
}
