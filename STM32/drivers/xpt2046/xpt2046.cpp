#include "xpt2046.h"
#include "system.h"

XPT2046::XPT2046(SPI& spi, Pin cs, Point2D size, uint16_t zThreshold):
    m_spi(spi), m_cs(cs), m_size(size), m_zThreshold(zThreshold)
{
    setCalibrationData(Point2D(0, 0), 1.f, 1.f);
}

bool XPT2046::init(Orientation orientation)
{
    m_cs.init(GPIO_MODE_OUTPUT_PP);
    m_cs.setHigh();

    setOrientation(orientation);

    return false;
}

bool XPT2046::readAll(Point2D& point, int16_t& z)
{
    m_cs.setLow();

    uint8_t out[] = {   0x00, 0x90/*0x98*/,
					    0x00, 0xD0/*0xD8*/,
					    0x00, 0x00};

	uint8_t in[sizeof(out)] =  {0};

	if( m_spi.write(0xB0, XPT2046::SPI_MAX_FREQUENCY) ||
	    m_spi.readWrite(reinterpret_cast<const uint8_t*>(&out), reinterpret_cast<uint8_t*>(&in), sizeof(out), XPT2046::SPI_MAX_FREQUENCY))
    {
        point   = Point2D();
        z       = 0;
        return true;
    }

	z           = ((static_cast<int16_t>(in[0])<<8) + in[1]) >> 3;
	point.x()   = ((static_cast<int16_t>(in[2])<<8) + in[3]) >> 3;
	point.y()   = ((static_cast<int16_t>(in[4])<<8) + in[5]) >> 3;
    
    m_cs.setHigh();

    return false;
}

std::pair<bool, Point2D> XPT2046::update()
{
    Point2D raw;
    int16_t z;
    
    if(System::millis() - m_lastUpdate < XPT2046::UPDATE_MIN_DELAY)
        return m_cachedResult;

    int16_t zSum = 0;

    for(int i=0; i<XPT2046::NUM_SAMPLES; ++i)
    {
        int16_t z;
        Point2D pts;
        readAll(pts, z);
        zSum += z;
        raw += pts;
    }

    z   /= XPT2046::NUM_SAMPLES;
    raw /= XPT2046::NUM_SAMPLES;

    if(readAll(raw, z) || z < m_zThreshold)    
        m_cachedResult = std::make_pair(false, Point2D());
    else
        m_cachedResult = std::make_pair(true, rawToReal(raw));

    m_lastUpdate = System::millis();
    return m_cachedResult;
}

Point2D XPT2046::rawToReal(Point2D raw) const
{
    raw -= m_minCorner;

    raw.x() = static_cast<int32_t>(m_stretchX*static_cast<float>(raw.x()*m_size.x())/RAW_MAX_X);
    raw.y() = static_cast<int32_t>(m_stretchY*static_cast<float>(raw.y()*m_size.y())/RAW_MAX_Y);

    switch(m_orientation)
    {
        case Orientation::PORTRAIT_2:
            raw.y() = m_size.y() - raw.y();
            std::swap(raw.x(), raw.y());
        break;
        case Orientation::PORTRAIT_1:
            raw.x() = m_size.x() - raw.x();
            std::swap(raw.x(), raw.y());
        break;
        case Orientation::LANDSCAPE_1:
            raw.x() = m_size.x() - raw.x();
            raw.y() = m_size.y() - raw.y();
        break;
        default:
        break;
    }

    return raw;
}

void XPT2046::setCalibrationData(Point2D minCorner, float x, float y)
{   
    Touch::setCalibrationData(minCorner, x, y);

    m_minCorner.x() = minCorner.x() * RAW_MAX_X / m_size.x();
    m_minCorner.y() = minCorner.y() * RAW_MAX_Y / m_size.y();
}

