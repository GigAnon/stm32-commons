/*
/version	4.0
/author		Arnaud CADOT
*/

#ifndef GUARD_PIN
#define GUARD_PIN

#include "hal.h"
#include "adc.h"

/**
\brief This class provides an easy-to-use interface for GPIOs.

This class provides an easy-to-use OO interface for GPIOs on STM32F4 hardware.
Pins can be set up as digital output or input. 
Subclasses can be made to provide support for ADCs, PWMs, etc.
\remark Instances of this class are lightweight wrappers.
They can be copied or destroyed without any issue or impact on the underlying hardware.
Essentialy, they can be seen as convenient pointers to the hardware GPIOs.
**/
class Pin
{
	public:
		/**
		\brief Constructor.
		\param port GPIO port of the pin
		\param pin Pin number
		
		\remark This method does not initialize anything, it just constructs the instance
		with the relevant data.
		\remark The pin number is the natural number (i.e. 0, 1) etc., and not the pin define
		(i.e. GPIO_Pin_0, GPIO_Pin_1, etc.)
		**/
		Pin(GPIO_TypeDef* port, uint8_t pin);
		
		/**
		\brief Default constructor.
		
		Default constructor. Represents an invalid pin.
		**/
		Pin();
		
		/**
		\brief Equality comparison operator
		\param other The pin to compare against
		\return \c true if and only if the two pins ports and number are identical.
		\remark Calls to initialization methods, setHigh, setLow, etc. have no effects on this
		operator. 
		**/
		bool operator==(const Pin& other) const;

        /**
        \brief Assignment operator
        \param other Pin to copy
        \return lvalue
        **/
        Pin& operator=(Pin& other);
	
		/**
		\returns \c true if the pin is valid, \c false otherwise.
		**/
		bool isValid() const;

        /**
        \brief Allows implicit conversions to boolean
		\returns \c true if the pin is valid, \c false otherwise.
		**/
        operator bool() const { return isValid(); }
	
		/**
		\returns The port of the GPIO
		**/
		GPIO_TypeDef* port() const;
	
		/**
		\returns The pin number of the GPIO
		\remark This is the natural pin number, i.e. '0' for PA0, '1' for PA1, etc.
		**/
		uint8_t pin() const;
	
		/**
		\returns The pin number of the GPIO, as defined by the HAL libraries
		\remark This numerotation is different from the natural numerotation, i.e. GPIO_Pin_N is often different from N.
		This was done to allow initialization/usage of multiple GPIOs at once, using bitwise operators.
		**/
		uint16_t definePin() const;
	
		/**
		\brief Invert the pin output.
		
		If the pin is set as digital output, invert its output, i.e. go from low to high, or high to low.
		Does nothing if the pin is not set, or set as digital input.
		**/
		void toggle();
	
		/**
		\brief Set pin output to high.
		
		If the pin is set as digital output, set it to high.
		Does nothing if the pin is not set, or set as digital input.
		**/
		void setHigh();
	
		/**
		\brief Set pin output to low.
		
		If the pin is set as digital output, set it to low.
		Does nothing if the pin is not set, or set as digital input.
		**/
		void setLow();
	
		/**
		\brief Set pin output.
		\param high If \c true, set pin output to high. Otherwise, set it to low.
		
		If the pin is set as digital output, set it to high or low.
		Does nothing if the pin is not set, or set as digital input.
		\see setHigh()
		\see setLow()
		**/
		void set(bool high);
		
		/**
		\returns The value (high or low) at the pin.
		
		This method returns the digital value at the GPIO, whether it is an output or an input.
		\remark The same limitations as isInput() are present here.
		**/
		bool read() const;
    
#if defined(HAL_ADC_MODULE_ENABLED)
        /**
        \brief Read value on ADC channels
        \param sampleTime ADC sample time
        \returns ADC value if available, 0xFFFF otherwise
        **/
        uint16_t adcRead(uint8_t sampleTime = ADCPeriph::DEFAULT_SAMPLE_TIME);
#endif

        /**
        \brief Initialize pin
        \param mode GPIO mode
        \param pull GPIO pull (up/down/none)
        \param speed GPIO speed
        \returns \c true on error, \c false otherwise
        **/
		bool init(uint32_t mode = GPIO_MODE_INPUT, uint32_t pull = GPIO_NOPULL, uint32_t speed = GPIO_SPEED_MEDIUM);
		
		/**
		\brief Translate a 'natural' pin numerotation into its corresponding HAL define
		**/
		static uint16_t pinNumToDefine(uint8_t num);
    
#if defined(HAL_ADC_MODULE_ENABLED)
        /**
        \param adc ADC instance
        \returns ADC channel associated to the pin, or 0xFF on error
        **/
        uint8_t getADCChannel(ADC_TypeDef* adc);
#endif

	protected:
		GPIO_TypeDef* const m_port;	///< GPIO port
		const uint8_t m_pin;		///< GPIO pin number
};


#endif
