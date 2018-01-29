#include "circular_buffer.h"

#include <cstdlib>

CircularBuffer::CircularBuffer(uint32_t size): m_bufferSize(size)
{
    m_buffer = reinterpret_cast<uint8_t*>(malloc(size));
}

CircularBuffer::~CircularBuffer()
{
    free(m_buffer);
}

uint32_t CircularBuffer::size() const
{
    if(m_end >= m_start)
		return m_end - m_start;
	
	return m_bufferSize + m_end - m_start;
}

void CircularBuffer::clear()
{
    m_start = m_end;
}

void CircularBuffer::incEnd()
{
    if(++m_end == m_bufferSize)
        m_end = 0;
}

void CircularBuffer::incStart()
{
    if(++m_start == m_bufferSize)
        m_start = 0;
}

bool CircularBuffer::push(uint8_t c)
{
    return push(&c, 1);
}

uint8_t CircularBuffer::pull()
{
    uint8_t c = 0x00;
    pull(&c, 1);
    return c;
}

bool CircularBuffer::push(uint8_t* data, uint32_t dataSize)
{
    bool overrun = false;

    for(uint32_t i=0;i<dataSize;++i)
    {
        m_buffer[m_end] = data[i];

        incEnd();

        if(m_end == m_start)
        {
            incStart();

            overrun = true;
        }
    }

    return overrun;
}
        
uint32_t CircularBuffer::pull(uint8_t* data, uint32_t maxData)
{
    uint32_t r = 0;

    while(m_start != m_end)
    {
        data[r] = m_buffer[m_start];

        incStart();

        ++r;
        if(r == maxData)
            break;
    }


    return r;
}
