/*
    \version	4.0
    \author		Arnaud CADOT
*/

#ifndef GUARD_I2C
#define GUARD_I2C

#include "hal.h"

#if defined(HAL_I2C_MODULE_ENABLED)

#include "byte_array.h"

/**
\brief I2C bus wrapper

This class provides a convenient wrapper over a master I2C bus, to simplify the use of most
(if not all) I2C slave periphericals.
Note this is a very barebones wrapper. Calls to read() are bloquing, and implementations may not
make any use of interrupts and/or DMA capabilities of the hardware.
Nonetheless, performances should be sufficient for most usages, even moderate external EEPROM access.
**/
class I2C
{
	public:
		static constexpr uint32_t DEFAULT_SPEED = 400000L;  ///< I2C bus speed, in bit/s
		static constexpr uint32_t TIMEOUT = 100L;           ///< I2C timeout for slave lookup
	
	public:
		/**
		\brief Constructor
		**/
		I2C(I2C_TypeDef* bus);
	
		/**
		\brief Initialization method
        \param speed I2C bus speed, in Hz. Default: I2C::DEFAULT_SPEED (400kHz)
		\returns \c true on error, \c false otherwise.
		
		This method initializes the I2C bus as a master.
		**/
		bool init(uint32_t speed = I2C::DEFAULT_SPEED);
	
		/**
		\brief 'Ping' an I2C slave at the specified address
		\param adr Right-aligned (i.e. normal) 7-bit address of the target slave device
		\returns \c true if a device answered, \c false otherwise
		
		\remark It is a good habit to check whether an I2C slave is connected, before
		trying to send/pull data from it, as otherwise you might wait some time for an
		answer that won't be coming, or get invalid data back.
		
		\remark I2C device addressing is 7-bits, left-aligned in the protocol specification, because
		the last bit of the command byte is used as R/W. However, using those addresses directly is
		very awkward, as it means they are all the even numbers in the [0-254] range.
		Instead, we use the 'natural' addressing, which mean all the numbers in the [0-127] range.
		This is how most I2C devices give their addresses in their datasheet.
		**/
		bool deviceAvailable(uint8_t adr);

        /**
        \brief Write data on bus
        \param devAdr Right-aligned 7-bit address of the target slave device
        \param data Data to be sent
        **/
        bool write(uint8_t devAdr, const ByteArray& data);

        /**
        \brief Write data on bus
        \param devAdr Right-aligned 7-bit address of the target slave device
        \param data Data to be sent
        \param dataSize Data size
        **/
        bool write(uint8_t devAdr, const uint8_t* data, uint16_t dataSize);
	
		/**
		\brief Write a single byte of data, in  8-bit adressed register, on a specific device
		\param devAdr Right-aligned 7-bit address of the target slave device
		\param regAdr A 8-bit register address to write on
		\param data Byte to write in the register
		\returns \c true on error, \c false otherwise
		**/
		bool write(uint8_t devAdr, uint16_t regAdr, uint8_t data);
	
		/**
		\brief Write some data, starting at a 8-bit adressed register, on a specific device
		\param devAdr Right-aligned 7-bit address of the target slave device
		\param regAdr A 8-bit register address to start writing on
		\param data Data to write
		\returns \c true on error, \c false otherwise
		**/
		bool write(uint8_t devAdr, uint16_t regAdr, const ByteArray& data);
	
		/**
		\brief Write NO data but 'poke' a 8-bit adressed register, on a specific device
		\param devAdr Right-aligned 7-bit address of the target slave device
		\param d A 8-bit register address of the register to 'poke'
		\returns \c true on error, \c false otherwise
		
		\remark Very often, I2C slave devices choose to take a register-based approch to handle
		the push/pull from an I2C master. Essentially, it means that every I2C 'payload' is
		lead by a register address (8 or 16 bits, depending on the device), followed by the data to
		write.
		However, it is possible that sometimes, this first byte is not used as a register address,
		but as a command to the device. Hence, this method allows sending only this byte ('poking'
		what would be otherwise a register).
		**/
		bool write(uint8_t devAdr, uint8_t d);
		
		/**
		\brief Write a single byte of data, in  16-bit adressed register, on a specific device
		\param devAdr Right-aligned 7-bit address of the target slave device
		\param regAdr A 16-bit register address to write on
		\param data Byte to write in the register
		\returns \c true on error, \c false otherwise
		**/
		bool write16(uint8_t devAdr, uint16_t regAdr, uint8_t data);
	
		/**
		\brief Write some data, starting at a 16-bit adressed register, on a specific device
		\param devAdr Right-aligned 7-bit address of the target slave device
		\param regAdr A 16-bit register address to start writing on
		\param data Data to write
		\returns \c true on error, \c false otherwise
		**/
		bool write16(uint8_t devAdr, uint16_t regAdr, const ByteArray& data);

        /**
        \brief Read data from a specific device
        \param devAdr Right-aligned 7-bit address of the target slave device
        \param data Data buffer to be written in
        \param dataSize Data size to be read
        \returns \c true on error, \c false otherwise

        \remark The read operation has a timeout specified by I2C::TIMEOUT
        \remark This is a barebone I2C read operation. For most I2C devices, read operations
        must be preceded by a write operation to set the read context (which register(s),
        data size...). Other read() overloads do this write operation internaly to read
        pseudo-registers.
        **/
        bool read(uint8_t devAdr, uint8_t* data, uint16_t dataSize);
		
		/**
		\brief Read a byte from a 8-bit addressed register on a specific device
		\param devAdr Right-aligned 7-bit address of the target slave device
		\param regAdr A 8-bit register address to read from
		\returns If possible, the byte read, 0 otherwise.
		**/
		uint8_t		read(uint8_t devAdr, uint16_t regAdr);
		
		/**
		\brief Read several bytes from a 8-bit addressed register on a specific device
		\param devAdr Right-aligned 7-bit address of the target slave device
		\param regAdr A 8-bit register address to start reading from
		\param size Maximum length of the data to be read
		\returns On error: ByteArray. Otherwise, a ByteArray of up to \c size bytes.
		
		\remark The read operation has a timeout specified by I2C::TIMEOUT. 
		**/
		ByteArray	read(uint8_t devAdr, uint16_t regAdr, uint16_t size);
	
		/**
		\brief Read a byte from a 16-bit addressed register on a specific device
		\param devAdr Right-aligned 7-bit address of the target slave device
		\param regAdr A 16-bit register address to read from
		\returns If possible, the byte read, 0 otherwise.
		**/
		uint8_t		read16(uint8_t devAdr, uint16_t regAdr);
	
		/**
		\brief Read several bytes from a 16-bit addressed register on a specific device
		\param devAdr Right-aligned 7-bit address of the target slave device
		\param regAdr A 16-bit register address to start reading from
		\param size Maximum length of the data to be read
		\returns On error: ByteArray. Otherwise, a ByteArray of up to \c size bytes.
		
		\remark The read operation has a timeout specified by I2C::TIMEOUT. 
		**/
		ByteArray	read16(uint8_t devAdr, uint16_t regAdr, uint16_t size);
	
	
	private:
		I2C_HandleTypeDef m_i2cHandle;

        void initPins();
};

#if defined(I2C1)
    extern I2C i2c1;
#endif
#if defined(I2C2)
    extern I2C i2c2;
#endif


#endif /* #if defined(HAL_I2C_MODULE_ENABLED) */
#endif /* #ifndef GUARD_I2C */