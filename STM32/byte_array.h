/*
/version	4.0
/author		Arnaud CADOT
*/

#ifndef GUARD_BYTE_ARRAY
#define GUARD_BYTE_ARRAY

#include <cstdint>
#include <initializer_list>

/**
\brief A simple yet flexible class to handle raw binary data.


The ByteArray class was created as a viable alternative to std::vector<uint8_t> (which is too heavy) and to
(Arduino) String (which is terrible and/or unavailable), when used as a raw array and/or buffer of bytes.
As such, it allows dynamic resizing, concatenation, copy, memory reservation, direct memory access, swap, etc.
It was also designed with stream operations in mind, where serializing data is often a source of headaches and/or bugs.
As such, it can write/read any simple types from/to a raw array of bytes.
Finally, the class provides a few other convenients methods, such as decoding/encoding from/to ASCII representation,
printing to Arduino streams, etc.

It manages an internal uint8_t buffer, reallocating it when strictly necessary and using modern C++11 move semantics
to keep superflous copies to a minimum.

Important note: on embedded devices, memory fragmentation may be a serious issue. Keep in mind any ByteArray instance may use
several dynamic allocations during its lifetime, and may end up all over the place in SRAM.
If you need a buffer/array, all the time, and you know the size of this buffer/array at compile time, consider using raw
C arrays instead.
*/
	class ByteArray
	{
		public:
			/**
				\brief Default constructor.
				\param size The size of the ByteArray. Defaults to 0.
				\param filler The value with which the array will be initialized. Defaults to 0x00.

				Create an array of size \c size, and fills it with \c filler, or an empty array.
				\remark O(size)
			**/
			ByteArray(std::size_t size = 0x00, uint8_t filler = 0x00);
			
			/**
				\brief Raw buffer constructor.
				\param buffer A valid uint8_t C-style array, from which the data will be copied.
				\param size The size of the buffer.

				Create an array of size \c size and copies the content of \c buffer in it.
				\remark O(size)
			**/
			ByteArray(const uint8_t* const buffer, std::size_t size);
			
			/**
				\brief C-string buffer constructor.
				\param cString A valid, null-terminated ('\0') C-style string, from which the data will be copied.

				Create an array of size \c strlen(size) and copies the content of \c cString in it.
				\remark O(strlen(cString))
				\remark Using a non-null terminated C-string is an undefined behavior.
				\remark Only the data leading up (and excluding) to the first null character ('\0') will be copied.
			**/
			ByteArray(const char* const cString);
				
			/**
				\brief Copy constructor.
				\param other Another ByteArray object to be copied

				Copies \c other. 
				\remark O(other.size())
			**/
			ByteArray(const ByteArray& other);
			
			/**
				\brief Move constructor.
				\param other The ByteArray to be moved

				Moves the contents of \c other into \c this.
				\remark O(1)
				\remark C++11
				\remark \c other will be left in an invalid state.
			**/
			ByteArray(ByteArray&& other);

            ByteArray(std::initializer_list<uint8_t> l);

			/**
				\brief Destructor.
			**/
			~ByteArray();

			/**
				\brief Assignement operator.
				\param other The ByteArray to be moved or copied
				\returns Reference to the current instance

				\remark O(1) if moved, O(other.size()) if copied
			**/
			ByteArray& operator=(ByteArray other);
		
			/**
				\brief Equality operator.
				\param other The ByteArray to be compared to
				\returns \c true on equality, \c false otherwise

				\remark O(n) maximum
			**/
			bool operator==(const ByteArray& other);
			
			/**
				\brief Inequality operator.
				\param other The ByteArray to be compared to
				\returns \c true on inequality, \c false otherwise

				\remark O(n) maximum
			**/
			bool operator!=(const ByteArray& other);

			/**
				\brief Self-concatenation operator.
				\param a The ByteArray to be concatenated to the end of the object
				\returns A reference to the present instance

				\remark O(a.size()), O(size() + a.size()) if a reallocation is needed
			**/
			ByteArray& operator+=(const ByteArray& a);

			/**
				\brief Concatenation operator.
				\param other The ByteArray to be concatenated.
				\returns A copy of the object, with \c a concatenated at the end of it.
				
				\remark O(size() + a.size())
			**/
			ByteArray operator+(const ByteArray& other) const;
			
			/**
				\brief Find a value in the array
				\param c The value to find
				\param startPos Starting offset
				\returns The index of the first matching value after startPos, or size() if not found

				\remark O(n)
			**/
			std::size_t find(uint8_t c, std::size_t startPos = 0) const;

			/**
				\brief Clear the array (resize to 0)

				\remark O(1)
				\remark Does not free the memory
				\see shrink
			**/
			void clear();
			
			/**
				\brief Resize the array, reallocate if necessary.
				\param size The new size of the array
				\param filler The value to be used for the newly created bytes at the end
				\remark O(1), O(size) if a reallocation if necessary.
				\see reserve
			**/
			void resize(std::size_t size, uint8_t filler = 0x00);
			
			/**
				\brief Resize the internal buffer, reallocate if necessary.
				\param size The new size of the buffer

				\remark O(1), O(size) if a reallocation if necessary.
				\remark Very useful when concatenating severy ByteArray of known size.
				\see resize
				\see shrink
			**/
			void reserve(std::size_t size);
			
			/**
				\brief Shrink the internal buffer to free the unused memory.
				\returns The new size of the internal buffer
				\remark O(1)
			**/
			std::size_t shrink();

			/**
				\returns The size of the array
				\remark O(1)
				\see bufferSize
			**/
			std::size_t size() const { return m_usedSize; }
			
			/**
				\returns The size of the internal buffer
				\remark O(1)
				\remark Important note: the bytes between size() and bufferSize() are reserved for the object to use, but should \
				NEVER be accessed directly. This function is provided to give the end-user an idea of the actual memory usage of \
				the object, and nothing more.
				\see size
			**/
			std::size_t bufferSize() const { return m_allocatedSize; }
			
			/**
				\returns A pointer to the internal buffer.

				Returns a pointer to the internal, C-style buffer. Should only be used with API requiring C-style buffers.

				\remark O(1)
				\remark NEVER free this pointer!
				\remark After resize/reserve/shrink operations, this pointer MAY be invalidated.
			**/
			uint8_t* internalBuffer() { return m_memory; }
			
			/**
				\returns A constant pointer to the internal buffer.
				\remark O(1)
				\remark NEVER free this pointer!
				\remark After resize/reserve/shrink operations, this pointer MAY be invalidated.
				\see internalBuffer()
			**/
			const uint8_t* internalBuffer() const { return m_memory; }

			/**
				\brief Access the byte at index idx
				\param idx The index of the byte to access.
				\returns Reference to the indexed byte

				Provide a C-style interface for the object.

				\remark O(1)
				\remark For optimization reasons, out of bounds indexes are undefined behavior.
			**/
			uint8_t& operator[](std::size_t idx);
			
			/**
				\brief Access the byte at index idx, constant version
				\param idx The index of the byte to access.
				\returns Constant reference to the indexed byte

				Provide a C-style (constant) interface for the object.

				\remark O(1)
				\remark For optimization reasons, out of bounds indexes are undefined behavior.
			**/
			const uint8_t& operator[](std::size_t idx) const;

			/**
				\brief Split a ByteArray in two, keep start and return end.
				\param idx The index after which the split will be effective
				\returns A ByteArray containing the second part

				Split the object in half at index \c idx, and returns the second half. 
				After this function returns, the object will only contain the first half.

				\remark O(size() - idx)
				\remark The object will be resized but memory will not be freed.
				\remark The second part HAS to be copied. It is not possible to just split the memory in two.
				\see shrink
				\see reverseSplit
			**/
			ByteArray split(std::size_t idx);
			
			/**
				\brief Split a ByteArray in two, keep end and return start.
				\param idx The index after which the split will be effective
				\returns A ByteArray containing the first part

				Functionnally similar to \c ByteArray::split, but swap its results.
				The object will contain the second (end) part, the returned ByteArray will contain the first (start) part.

				\remark O(size() - idx)
				\remark This is a split() followed by a swap(), so all notes in split() applies here, although in reverse.
				\see shrink
				\see split
			**/
			ByteArray reverseSplit(std::size_t idx);

			/**
				\param other The ByteArray to be compared with
				\returns true if the object starts with other, false otherwise.
				\remark O(other.size())
			**/
			bool startsWith(const ByteArray& other) const;

			/**
				\param other The ByteArray to swap the object with
				\remark O(1)
			**/
			void swap(ByteArray& other);

			/**
				\brief Append a single byte to the object
				\param c The byte to append
				\returns The new size of the array

				Append a single byte to the array. If necessary, reallocate the internal buffer.
				\remark O(1) amortized - worst case is O(object.size())
				\remark Amortisation factor is very low on this implementation, because SRAM is at a premium here.
				\remark When adding a known count of bytes, please use the self-concatenation operator instead
			**/
			std::size_t append(uint8_t c);
			
			/**
				\brief Convert the ByteArray to an human-friendly hex format.
				\param separator The character to use to separate pairs of nibbles. Set to 0 for none.
				\returns The hex-encoded result
			**/
			ByteArray asHex(uint8_t separator = 0) const;
			
			/**
				\brief Interpret the ByteArray as integer written in ASCII.
				\returns 0 on error, the integer value otherwise
				\remark This is essentially a sscanf(buf, "%ld", i) and has the same limitations. 
			**/
			int32_t asInt() const;

			/**
				\brief Create a ByteArray from data in ASCII representation
				\param s The data to translate
				\returns Raw, translated data in a ByteArray

				\remark O(s.size())
			**/
			static ByteArray fromHex(const ByteArray& s);
		
			/**
				\brief Create a ByteArray from a generic naive type
				\param d The value to serialize
				\param bigEndian Set to \c false to use Little Endian convention
				\returns Data serialized in a ByteArray
			**/
			template<typename T>
			static ByteArray serialize(const T& d, bool bigEndian = true);

			/**
				\brief Read a generic naive type from the object
				\param idx The index of the value to read
				\returns Object read, or default-constructed object in case of failure

				\todo Add Little Endian support
			**/
			template<typename T>
			T read(std::size_t idx = 0) const;

		private:
			uint8_t* m_memory;

			std::size_t m_allocatedSize;
			std::size_t m_usedSize;
	};


template<typename T>
ByteArray ByteArray::serialize(const T& d, bool bigEndian)
{
	// Important note: AVR and ARM architectures uses LITTLE ENDIAN notation.
	// x86 architecture (often) use BIG ENDIAN notation.
	// This code is designed to run on an AtMega (AVR) or a STM32F4 (ARM) chip, hence it
	// assumes the processor runs in little endian convention.
	if(bigEndian)
	{
		ByteArray b(sizeof(T), 0x00);

		uint16_t bsft = 0;

		for(std::size_t i=0; i < sizeof(T); ++i)
		{
			b[sizeof(T) - i - 1] = ((d >> bsft) & 0xFF);
			bsft += 8;
		}

		return b;
	}
	else
		return ByteArray(reinterpret_cast<const uint8_t* const>(&d), sizeof(T));
}

template<typename T>
T ByteArray::read(std::size_t idx) const
{
	// Little Endian. Maybe add Big Endian latter?
	if(size() - idx < sizeof(T))
		return T();

	T r = 0;

	for (std::size_t i = 0; i < sizeof(T); ++i)
		r += (static_cast<T>(operator[](idx + sizeof(T) - 1 - i)) << (i*8));

	return r;
}

#endif
