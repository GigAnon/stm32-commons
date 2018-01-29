/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <cmath>

#include "bme280.h"


BME280::BME280(I2C& i2c): m_i2c(i2c)
{}

bool BME280::init(uint8_t a)
{
    m_i2cAddr = a;

    if (!m_i2c.deviceAvailable(m_i2cAddr) || read8(Registers::CHIPID) != 0x60)
        return true;

    readCoefficients();

    //Set before CONTROL_meas (DS 5.4.3)
    write8(Registers::CONTROLHUMID, 0x05); //16x oversampling 

    write8(Registers::CONTROL, 0xB7); // 16x ovesampling, normal mode

    m_initOK = true;

    return false;
}


/**************************************************************************/
/*!
    @brief  Writes an 8 bit value over I2C
*/
/**************************************************************************/
void BME280::write8(uint8_t reg, uint8_t value)
{
	m_i2c.write(m_i2cAddr, reg, value);
}

/**************************************************************************/
/*!
    @brief  Reads an 8 bit value over I2C
*/
/**************************************************************************/
uint8_t BME280::read8(uint8_t reg)
{
	return m_i2c.read(m_i2cAddr, reg);
}

/**************************************************************************/
/*!
    @brief  Reads a 16 bit value over I2C
*/
/**************************************************************************/
uint16_t BME280::read16(uint8_t reg)
{
	ByteArray a(m_i2c.read(m_i2cAddr, reg, 2));
	
	if(a.size() != 2)
		return 0x00;
	
	return a.read<uint16_t>(0);
}

uint16_t BME280::read16_LE(uint8_t reg)
{
	uint16_t temp = read16(reg);
	return (temp >> 8) | (temp << 8);
}

/**************************************************************************/
/*!
    @brief  Reads a signed 16 bit value over I2C
*/
/**************************************************************************/
int16_t BME280::readS16(uint8_t reg)
{
	return (int16_t)read16(reg);
}

int16_t BME280::readS16_LE(uint8_t reg)
{
	return (int16_t)read16_LE(reg);
}


/**************************************************************************/
/*!
    @brief  Reads a 24 bit value over I2C
*/
/**************************************************************************/

uint32_t BME280::read24(uint8_t reg)
{
	ByteArray a(1, 0x00);
	a += m_i2c.read(m_i2cAddr, reg, 3);
	
	if(a.size() != 4)
		return 0x00;
	
	return a.read<uint32_t>(0);
}


/**************************************************************************/
/*!
    @brief  Reads the factory-set coefficients
*/
/**************************************************************************/
void BME280::readCoefficients(void)
{
    m_calibrationData.dig_T1 = read16_LE(Registers::DIG_T1);
    m_calibrationData.dig_T2 = readS16_LE(Registers::DIG_T2);
    m_calibrationData.dig_T3 = readS16_LE(Registers::DIG_T3);

    m_calibrationData.dig_P1 = read16_LE(Registers::DIG_P1);
    m_calibrationData.dig_P2 = readS16_LE(Registers::DIG_P2);
    m_calibrationData.dig_P3 = readS16_LE(Registers::DIG_P3);
    m_calibrationData.dig_P4 = readS16_LE(Registers::DIG_P4);
    m_calibrationData.dig_P5 = readS16_LE(Registers::DIG_P5);
    m_calibrationData.dig_P6 = readS16_LE(Registers::DIG_P6);
    m_calibrationData.dig_P7 = readS16_LE(Registers::DIG_P7);
    m_calibrationData.dig_P8 = readS16_LE(Registers::DIG_P8);
    m_calibrationData.dig_P9 = readS16_LE(Registers::DIG_P9);

    m_calibrationData.dig_H1 = read8(Registers::DIG_H1);
    m_calibrationData.dig_H2 = readS16_LE(Registers::DIG_H2);
    m_calibrationData.dig_H3 = read8(Registers::DIG_H3);
    m_calibrationData.dig_H4 = (read8(Registers::DIG_H4) << 4) | (read8(Registers::DIG_H4+1) & 0xF);
    m_calibrationData.dig_H5 = (read8(Registers::DIG_H5+1) << 4) | (read8(Registers::DIG_H5) >> 4);
    m_calibrationData.dig_H6 = (int8_t)read8(Registers::DIG_H6);
}

float BME280::readTemperature(void)
{
    int32_t var1, var2;

    int32_t adc_T = read24(Registers::TEMPDATA);
    adc_T >>= 4;

    var1  = ((((adc_T>>3) - ((int32_t)m_calibrationData.dig_T1 <<1))) *
	    ((int32_t)m_calibrationData.dig_T2)) >> 11;

    var2  = (((((adc_T>>4) - ((int32_t)m_calibrationData.dig_T1)) *
	        ((adc_T>>4) - ((int32_t)m_calibrationData.dig_T1))) >> 12) *
	    ((int32_t)m_calibrationData.dig_T3)) >> 14;

    t_fine = var1 + var2;

    float T  = (t_fine * 5 + 128) >> 8;
    return T/100;
}

uint16_t BME280::readTemperature16bits()
{
	return (uint16_t)(readTemperature()*100.f); // Watch out for underflow
}

float BME280::readPressure()
{
    int64_t var1, var2, p;

    readTemperature(); // must be done first to get t_fine

    int32_t adc_P = read24(Registers::PRESSUREDATA);
    adc_P >>= 4;

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)m_calibrationData.dig_P6;
    var2 = var2 + ((var1*(int64_t)m_calibrationData.dig_P5)<<17);
    var2 = var2 + (((int64_t)m_calibrationData.dig_P4)<<35);
    var1 = ((var1 * var1 * (int64_t)m_calibrationData.dig_P3)>>8) +
    ((var1 * (int64_t)m_calibrationData.dig_P2)<<12);
    var1 = (((((int64_t)1)<<47)+var1))*((int64_t)m_calibrationData.dig_P1)>>33;

    if (var1 == 0)
        return 0;
  
    p = 1048576 - adc_P;
    p = (((p<<31) - var2)*3125) / var1;
    var1 = (((int64_t)m_calibrationData.dig_P9) * (p>>13) * (p>>13)) >> 25;
    var2 = (((int64_t)m_calibrationData.dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)m_calibrationData.dig_P7)<<4);
    return (float)p/256;
}

uint16_t BME280::readPressure16bits()
{
	return (uint16_t)(readPressure()/10.f);
}

float BME280::readHumidity()
{
    readTemperature(); // must be done first to get t_fine

    int32_t adc_H = read16(Registers::HUMIDDATA);

    int32_t v_x1_u32r;

    v_x1_u32r = (t_fine - ((int32_t)76800));

    v_x1_u32r = (((((adc_H << 14) - (((int32_t)m_calibrationData.dig_H4) << 20) -
		    (((int32_t)m_calibrationData.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
	        (((((((v_x1_u32r * ((int32_t)m_calibrationData.dig_H6)) >> 10) *
		    (((v_x1_u32r * ((int32_t)m_calibrationData.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
		    ((int32_t)2097152)) * ((int32_t)m_calibrationData.dig_H2) + 8192) >> 14));

    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
			        ((int32_t)m_calibrationData.dig_H1)) >> 4));

    v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
    v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
    float h = (v_x1_u32r>>12);
    return  h / 1024.0;
}

uint16_t BME280::readHumidity16bits()
{
	return static_cast<uint16_t>(readHumidity()*100);
}

/**************************************************************************/
/*!
    Calculates the altitude (in meters) from the specified atmospheric
    pressure (in hPa), and sea-level pressure (in hPa).

    @param  seaLevel      Sea-level pressure in hPa
*/
/**************************************************************************/
float BME280::readAltitude(float seaLevel)
{
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using this equation from Wikipedia can give bad results
  // at high altitude. See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

  float atmospheric = readPressure() / 100.0F;
  return 44330.0 * (1.0 - pow(atmospheric / seaLevel, 0.1903));
}

/**************************************************************************/
/*!
    Calculates the pressure at sea level (in hPa) from the specified altitude 
    (in meters), and atmospheric pressure (in hPa).  
    @param  altitude      Altitude in meters
    @param  atmospheric   Atmospheric pressure in hPa
*/
/**************************************************************************/
float BME280::seaLevelForAltitude(float altitude, float atmospheric)
{
    // Equation taken from BMP180 datasheet (page 17):
    //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

    // Note that using the equation from wikipedia can give bad results
    // at high altitude.  See this thread for more information:
    //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064
  
    return atmospheric / pow(1.0 - (altitude/44330.0), 5.255);
}
