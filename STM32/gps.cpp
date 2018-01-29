#include "gps.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "system.h"
#include "datetime.h"


GPS::GPSData::GPSData(float latitude, float longitude, float speed, float heading, float altitude,
				float horizontalAccuracy, uint32_t timestamp)
{
	this->latitude				= latitude;
	this->longitude				= longitude;
	this->horizontalAccuracy	= horizontalAccuracy;
	this->timestamp				= timestamp;
	this->speed					= speed;
	this->heading				= heading;
	this->altitude				= altitude;
}

GPS::GPSData::GPSData():GPSData(100.0, 100.0)
{}

bool GPS::GPSData::isValid() const
{
	return (this->latitude<=90.0 && this->latitude>=-90.0 &&
			this->longitude<=90.0 && this->longitude>=-90.0 &&
			this->horizontalAccuracy >= 0.0);
}

float GPS::GPSData::distanceTo(const GPSData& other) const
{
	/*
	Using 'Haversine' formula
	Note: Cortex M4 series (used in STM32F4xxx) only have a single-precision
	FPU. It means double support is software only.
	A quick benchmark showed that using double precision was 4 to 5 times slower
	and added no accuracy to the result.
	See:
	http://www.movable-type.co.uk/scripts/latlong.html
	*/
	if(!isValid() || !other.isValid())
		return -1.f;
	
	constexpr double pi = 3.14159265359;
	constexpr double r = 6371.f; // Earth mean radius in km
	
	float lat1 = latitude*pi/180.f;
	float lon1 = longitude*pi/180.f;
	float lat2 = other.latitude*pi/180.f;
	float lon2 = other.longitude*pi/180.f;
	
	float a = std::pow(std::sin((lat2-lat1)/2),2) +
			std::cos(lat1)*std::cos(lat2)*
			std::pow(std::sin((lon2-lon1)/2),2);
	float c = 2*std::atan2(std::sqrt(a), std::sqrt(1-a));
	
	return r*c*1000;
}


GPS::GPS(Serial& serial, Pin resetPin, Pin powerPin):
	m_serial(serial), m_resetPin(resetPin)
{}

bool GPS::init()
{
	clearFix();
	if(m_resetPin.isValid())
		m_resetPin.init(GPIO_MODE_OUTPUT_PP);
		
	HAL_Delay(100);
	reset();
		
	m_serial.clear();
	
	//send("PMTK314,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
}

bool GPS::update()
{
    return updateRx();
}

bool GPS::updateRx()
{
	if(!m_serial.dataAvailable())
		return false;
	
	m_serial.readAppend(m_buffer);
	
	uint32_t t = m_currentFix.timestamp;
		
	while(true)
	{
		size_t idx = m_buffer.find('\n');
	
		if(idx == m_buffer.size())
			return m_currentFix.timestamp != t;
		
		ByteArray line = m_buffer.reverseSplit(idx+1);

        line.append(0);
					
		if(line.startsWith("$GPGGA"))
			processGPGGA(line);
		else if(line.startsWith("$GPRMC"))
        {
        //    printf("%s", line.internalBuffer());
			processGPRMC(line);
        }
		else if(line.startsWith("$GPVTG"))
			processGPVTG(line);
	}
		
	return m_currentFix.timestamp != t;
}

void GPS::processGPGGA(ByteArray& d)
{
	uint32_t pos = 0;
	
	CardinalPoint latCard = CardinalPoint::N;
	CardinalPoint lonCard = CardinalPoint::E;
		
	for(uint32_t pos=0;pos<14;++pos)
	{
		size_t idx = d.find(',');
		if(idx == d.size())
			break;
		
		ByteArray field = d.reverseSplit(idx+1);
		field[field.size()-1] = '\0';
		
		
		switch(pos)
		{
			case 1:
			// Time
			break;
			case 2:
			// Latitude num
			{
				float b;
				sscanf((const char*)field.internalBuffer(), "%f", &b);
				int deg = (int)(b/100.f);
				float mins = b - (float)(deg*100);
				m_currentFix.latitude = (float)deg + mins/60.f;
				
				if(lonCard == CardinalPoint::S)
					m_currentFix.latitude *= -1;
			}
			break;
			case 3:
				// Latitude N/S
				if(field[0] == 'N')
					latCard = CardinalPoint::N;
				else if(field[0] == 'S')
					latCard = CardinalPoint::S;
				
				m_currentFix.latitude = ((latCard == CardinalPoint::S)?-1:1)*std::fabs(m_currentFix.latitude);
			break;
			case 4:
			// Longitude num
			{
				float b;
				sscanf((const char*)field.internalBuffer(), "%f", &b);
				int deg = (int)(b/100.f);
				float mins = b - (float)(deg*100);
				m_currentFix.longitude = (float)deg + mins/60.f;
				
				if(lonCard == CardinalPoint::W)
					m_currentFix.longitude *= -1;
			}
			break;
			case 5:
				// Longitude W/E
				if(field[0] == 'E')
					lonCard = CardinalPoint::E;
				else if(field[0] == 'W')
					lonCard = CardinalPoint::W;
			
				m_currentFix.longitude = ((lonCard == CardinalPoint::W)?-1:1)*std::fabs(m_currentFix.longitude);
			break;
			case 6:
			if(field[0] == 0)
				return;
			break;
			case 7:
			// Sats in view
			break;
			case 8:
			// Horizontal accuracy
			sscanf((const char*)field.internalBuffer(), "%f", &m_currentFix.horizontalAccuracy);
			break;
			case 9:
			// Altitude
			sscanf((const char*)field.internalBuffer(), "%f", &m_currentFix.altitude);
			break;
			default:
			// Useless crap & checksum
			break;
		}
	}
}

void GPS::processGPRMC(ByteArray& d)
{
	uint32_t pos = 0;

    DateTime::Date date;
    DateTime::Time time;
	bool gotDate=false;
	bool gotTime=false;
	
	CardinalPoint latCard = CardinalPoint::N;
	CardinalPoint lonCard = CardinalPoint::E;
				
	for(uint32_t pos=0;pos<15;++pos)
	{
		size_t idx = d.find(',');
		if(idx == d.size())
			break;
		
		ByteArray field = d.reverseSplit(idx+1);
		field[field.size()-1] = '\0';
		
		switch(pos)
		{
			case 1:
			// Time
			{
				uint32_t t = 0;
				if(sscanf((const char*)field.internalBuffer(), "%lu", &t) == 1)
				{
					gotTime = true;
					time.h = t/10000;
					time.m = t/100 - time.h*100;
					time.s = t - time.m * 100 - time.h*10000;
				}
			}
			break;
			case 2:
			if(field[0] != 'A')
				return;
			break;
			case 3:
			// Latitude num
			{
				float b;
				if(sscanf((const char*)field.internalBuffer(), "%f", &b) == 1)
				{
					int deg = (int)(b/100.f);
					float mins = b - (float)(deg*100);
					m_currentFix.latitude = (float)deg + mins/60.f;
				
					if(lonCard == CardinalPoint::S)
						m_currentFix.latitude *= -1;
				}
			}
			break;
			case 4:
				// Latitude N/S
				if(field[0] == 'N')
					latCard = CardinalPoint::N;
				else if(field[0] == 'S')
					latCard = CardinalPoint::S;
				
				m_currentFix.latitude = ((latCard == CardinalPoint::S)?-1:1)*std::fabs(m_currentFix.latitude);
			break;
			case 5:
				// Longitude num
				{
					float b;
					if(sscanf((const char*)field.internalBuffer(), "%f", &b) == 1)
					{
						int deg = (int)(b/100.f);
						float mins = b - (float)(deg*100);
						m_currentFix.longitude = (float)deg + mins/60.f;
				
						if(lonCard == CardinalPoint::W)
							m_currentFix.longitude *= -1;
					}
				}
				break;
			case 6:
				// Longitude W/E
				if(field[0] == 'E')
					lonCard = CardinalPoint::E;
				else if(field[0] == 'W')
					lonCard = CardinalPoint::W;
			
				m_currentFix.longitude = ((lonCard == CardinalPoint::W)?-1:1)*std::fabs(m_currentFix.longitude);
			case 7:
			// Speed in knots
			{
				float f;
				if(sscanf((const char*)field.internalBuffer(), "%f", &f) == 1)
					m_currentFix.speed = f*1.852;
			}
			break;
			case 8:
			// Heading
			{
				float f;
				if(sscanf((const char*)field.internalBuffer(), "%f", &f) == 1)
					m_currentFix.heading = f;
			}
			break;
			case 9:
			// Date
			{
				uint32_t t = 0;
				if(sscanf((const char*)field.internalBuffer(), "%lu", &t) == 1)
				{
					gotDate = true;
					date.d = t/10000;
					date.m = t/100 - date.d*100;
					date.y = t - time.m * 100 - date.d*10000;
				}
			}
			break;
			default:
			// Useless crap & checksum
			break;
		}
	}
	
	uint32_t epoch = DateTime::computeEpochFromDateTime(date, time);
	if(epoch && gotDate && gotTime)
	{	
		m_currentFix.timestamp = epoch;

        printf("Got datetime! %d/%d/%d %d:%d:%d\n",
                date.d, date.m, date.y,
                time.h, time.m, time.s);

		/*if(std::max(clock.secondsSinceEpoch(), epoch) - std::min(clock.secondsSinceEpoch(), epoch) > 5)
		{
			debugln("Updating RTC with GPS time");
			debug(epoch);
			debug(" ");
			debugln(clock.secondsSinceEpoch());
			clock.setSecondsSinceEpoch(epoch);
			serialProtocol.sendRTCTime();
			
			debug(time.Day);
			debug("/");
			debug(time.Month);
			debug("/");
			debug(time.Year);
			debug(" ");
			debug(time.Hours);
			debug(":");
			debug(time.Minutes);
			debug(":");
			debugln(time.Seconds);
		}*/
	}
}

void GPS::processGPVTG(ByteArray& d)
{
	uint32_t pos = 0;
		
	for(uint32_t pos=0;pos<15;++pos)
	{
		size_t idx = d.find(',');
		if(idx == d.size())
			break;
		
		ByteArray field = d.reverseSplit(idx+1);
		field[field.size()-1] = '\0';
		
		
		switch(pos)
		{
			case 1:
			// Heading
			sscanf((const char*)field.internalBuffer(), "%f", &m_currentFix.heading);
			break;
			case 7:
			// Speed in km/h
			sscanf((const char*)field.internalBuffer(), "%f", &m_currentFix.speed);
			break;
			default:
			// Useless crap & checksum
			break;
		}
	}
}

bool GPS::reset()
{
	if(!m_resetPin.isValid())
		return true;
	
	m_resetPin.setLow();
	HAL_Delay(100);
	m_resetPin.setHigh();
	HAL_Delay(100);
	
	return false;
}

bool GPS::hasFix() const
{
	return m_currentFix.isValid() && m_currentFix.timestamp != 0;
}

void GPS::clearFix()
{
	m_currentFix = GPSData();
}

GPS::GPSData GPS::getFix() const
{
	return m_currentFix;
}

void GPS::send(const ByteArray& data)
{
	ByteArray d;
	d.reserve(data.size() + 6);
	
	d.append('$');
	d += data;
	d.append('*');
	
	uint8_t checksum = 0x00;
	for(size_t i=0;i<data.size();++i)
		checksum ^= data[i];
	
	char buff[5] = {0};
	snprintf(buff, 5, "%02x", checksum);
	
	d += ByteArray(buff);
	
	d.append('\r');
	d.append('\n');
	
	m_serial.write(d);
}