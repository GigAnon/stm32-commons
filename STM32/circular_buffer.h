#ifndef GUARD_CIRCULAR_BUFFER
#define GUARD_CIRCULAR_BUFFER

#include <cstdint>

/**
\brief A basic, fixed-size circular buffer class.

CircularBuffer is a bare-bone circular buffer class.
Memory is allocated on the heap and cannot be reallocated during usage.
**/
class CircularBuffer
{
    public:
        /**
        \brief Constructor.
        \param size Buffer size, in bytes.
        **/
        CircularBuffer(uint32_t size);

        /** Copy constructor **/
        CircularBuffer(CircularBuffer&) = delete;

        /** Move constructor **/
        CircularBuffer(CircularBuffer&&) = delete;

        /**
        Destructor.
        **/
        ~CircularBuffer();

        /**
        \returns Data available in the buffer, in bytes
        **/
        uint32_t size() const;

        /**
        \brief Clear (empty) the buffer
        **/
        void clear();

        /**
        \brief Add data to the buffer
        \param data Data to be added
        \param dataSize Size of the data to be added
        \returns \c true if the buffer is overrun, \c false otherwise
        **/
        bool push(uint8_t* data, uint32_t dataSize);

        /**
        \brief Add a byte to the buffer
        \param c Byte to be added
        \returns \c true if the buffer is overrun, \c false otherwise
        **/
        bool push(uint8_t c);
        
        /**
        \brief Read data from the buffer, up to min(maxData, size())
        \param data Destination buffer
        \param maxData Max data to be read
        \returns Bytes read
        **/
        uint32_t pull(uint8_t* data, uint32_t maxData);

        /**
        \brief Read a byte from the buffer
        \returns The byte read, or 0 on error
        **/
        uint8_t pull();

    private:
        const uint32_t m_bufferSize;
        uint8_t* m_buffer = nullptr;

        uint32_t m_start = 0;
        uint32_t m_end   = 0;

        void incEnd();
        void incStart();
};


#endif
