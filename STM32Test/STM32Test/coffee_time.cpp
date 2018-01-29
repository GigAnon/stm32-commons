#include "coffee_time.h"
#include "system.h"

#include <algorithm>
#include <cstring>

CoffeeTime::CoffeeTime(SSD1306& screen): m_screen(screen)
{}

void CoffeeTime::init()
{
    m_screen.init(SSD1306::SupplyType::Internal);
    m_screen.clear();
}

void CoffeeTime::update()
{
    if(System::millis() - m_lastRefresh > REFRESH_DELAY)
    {
        m_screen.clear();
        m_screen.drawLine(0, 55, 127, 55, 1);

        size_t s = std::strlen(CRAWLER_TEXT);

        for (size_t i = 0; i < std::min(s, (unsigned)(128 / 6)); ++i)
			m_screen.drawChar(i * 6, 7, CRAWLER_TEXT[(i + m_charIdx) % s]);

        m_screen.drawString(10, 0,"Power (Watts)");

        float current = (MAX_CURRENT * CALIB_FACTOR * ANALOG_REF * m_value) / (LOOPS*1.4142*4096.f);

        m_screen.drawMultipleSegments(10, 17, current*MAINS_VOLTAGE, 32);

		if(m_value > 200)
		{ 
			m_screen.drawCircle(3, 52 - m_bally, 3, 1);

			m_bally += m_ballSpeed;
			--m_ballSpeed;

			if(m_bally <= 0)
			{
				m_bally = 0;
				m_ballSpeed = 8;
			}
		}

		if(m_connected)
			m_screen.drawChar(112, 0, 2);
		else
			m_screen.drawChar(112, 0, 1);

        m_screen.display();
        m_lastRefresh = System::millis();
    }

    if(System::millis() - m_lastCharShift > CHAR_SHIFT_DELAY)
    {
        ++m_charIdx;
        m_lastCharShift = System::millis();
    }
}

void CoffeeTime::splashscreen()
{
    m_screen.clear();
	m_screen.drawString(32, 7, SPLASHSCREEN_TEXT);

	uint16_t o_x = (128-32)/2;
	uint16_t o_y = 2;

	uint8_t x[] = {0,31,31,0};
	uint8_t y[] = {0,15,48,17};

	float a_1 = (float(y[1]) - float(y[0]))/(float(x[1]) - float(x[0]));
	float b_1 = 0;

	float a_2 = (float(y[3]) - float(y[2]))/(float(x[3]) - float(x[2]));;
	float b_2 = float(y[3]) + 1;

	for(int i=0;i<4;++i)
		m_screen.drawLine(o_x+x[i], o_y+y[i], o_x+x[(i+1)%4], o_y+y[(i+1)%4], 1);

	m_screen.display();

    uint32_t t = System::millis();

	for (int i=0; i<x[1]; ++i)
	{
		m_screen.drawLine(o_x+i, o_y+a_1*i+b_1, o_x+i, o_y+a_2*i+b_2, 1);
		m_screen.display();

    	System::delay(System::millis() - t + SPLASHSCREEN_FRAME_DELAY);

        t = System::millis();
	}

    m_screen.display();
    
    System::delay(SPLASHSCREEN_DELAY);
}

void CoffeeTime::setValue(uint16_t value)
{
    m_value = value;
}

void CoffeeTime::setConnected(bool connected)
{
	m_connected = connected;
}

