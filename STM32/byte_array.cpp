#include "byte_array.h"

/* For memset, memcpy */
#include <cstring>
/* For malloc, realloc, free */
#include <cstdlib>
/* For snprintf, sscanf */
#include <cstdio>

using std::size_t;

ByteArray::ByteArray(std::size_t size, uint8_t filler)
{
	m_usedSize = size;
	m_allocatedSize = m_usedSize * 2;

	if(m_allocatedSize < 5)
		m_allocatedSize = 5;

	m_memory = reinterpret_cast<uint8_t*>(malloc(m_allocatedSize));

	memset(m_memory, filler, m_usedSize);
}

ByteArray::ByteArray(const uint8_t* const buffer, std::size_t size)
{
	m_usedSize = size;
	m_allocatedSize = m_usedSize;

	m_memory = reinterpret_cast<uint8_t*>(malloc(m_allocatedSize));

	memcpy(m_memory, buffer, size);
}

#ifdef ALLOW_ARDUINO_STRINGS
ByteArray::ByteArray(const String& string): ByteArray(string.c_str())
{}
#endif

ByteArray::ByteArray(const char* const cString): ByteArray(reinterpret_cast<const uint8_t*>(cString), strlen(cString))
{}

ByteArray::ByteArray(const ByteArray& other): ByteArray(other.m_memory, other.m_usedSize)
{}

ByteArray::ByteArray(std::initializer_list<uint8_t> l)
{
    m_usedSize = l.size();
	m_allocatedSize = m_usedSize;

	m_memory = reinterpret_cast<uint8_t*>(malloc(m_allocatedSize));

    auto m = m_memory;

    for(auto&& i:l)
        *(m++) = i;
}

bool ByteArray::startsWith(const ByteArray& other) const
{
	if(other.size() > size())
		return false;

	for(size_t i=0;i<other.size();++i)
		if(m_memory[i] != other[i])
			return false;

	return true;
}

std::size_t ByteArray::find(uint8_t c, std::size_t startPos) const
{	
	for(std::size_t i=startPos;i<size();++i)
		if(m_memory[i] == c)
			return i;
	
	return size();
}

ByteArray::ByteArray(ByteArray&& other)
{
	m_usedSize = other.m_usedSize;
	m_allocatedSize = other.m_allocatedSize;
	m_memory = other.m_memory;

	other.m_memory = nullptr;
}

ByteArray::~ByteArray()
{
	if(m_memory != nullptr)
		free(m_memory);
}

ByteArray& ByteArray::operator=(ByteArray other)
{
	swap(other);

	return *this;
}

ByteArray& ByteArray::operator+=(const ByteArray& a)
{
	reserve(m_usedSize + a.size());

	memcpy(m_memory+m_usedSize, a.m_memory, a.size());

	m_usedSize = m_usedSize + a.size();

	return *this;
}

ByteArray ByteArray::operator+(const ByteArray& other) const
{
	ByteArray t(*this);

	t += other;

	return t;
}

bool ByteArray::operator==(const ByteArray& other)
{
	if(other.size() != size())
		return false;
	
	for(size_t i=0;i<size();++i)
		if(operator[](i) != other[i])
			return false;
	
	return true;
}

bool ByteArray::operator!=(const ByteArray& other)
{
	return !((*this)==other);
}

void ByteArray::clear()
{
	resize(0);
}

void ByteArray::resize(std::size_t size, uint8_t filler)
{
	if(m_usedSize == size)
		return;

	if(m_usedSize > size)
	{
		m_usedSize = size;
		return;
	}

	reserve(size);

	memset(m_memory + m_usedSize, filler, size - m_usedSize);
	m_usedSize = size;
}

void ByteArray::reserve(std::size_t size)
{
	if (m_allocatedSize < size)
	{
		m_memory = reinterpret_cast<uint8_t*>(realloc(m_memory, size));
		m_allocatedSize = size;
	}
}

size_t ByteArray::shrink()
{
	m_allocatedSize = m_usedSize;

	if(m_allocatedSize < 5)
		m_allocatedSize = 5;

	m_memory = reinterpret_cast<uint8_t*>(realloc(m_memory, m_allocatedSize));

	return m_allocatedSize;
}

uint8_t& ByteArray::operator[](std::size_t idx)
{
	return m_memory[idx];
}

const uint8_t& ByteArray::operator[](std::size_t idx) const
{
	return m_memory[idx];
}

ByteArray ByteArray::split(std::size_t idx)
{
	if (idx > m_usedSize)
		return ByteArray();

	if(idx == m_usedSize)
	{
		resize(idx);
		return ByteArray();
	}

	ByteArray end(m_memory + idx, m_usedSize - idx);

	resize(idx);

	return end;
}

ByteArray ByteArray::reverseSplit(std::size_t idx)
{
	ByteArray t(split(idx));
	swap(t);
	return t;
}

#define swp(a, b) { decltype(a) t = a; a = b; b = t; }

void ByteArray::swap(ByteArray& other)
{
	swp(m_allocatedSize, other.m_allocatedSize);
	swp(m_memory, other.m_memory);
	swp(m_usedSize, other.m_usedSize);
}

size_t ByteArray::append(uint8_t c)
{
	if(m_usedSize == m_allocatedSize)
		reserve(m_usedSize + 8);

	m_memory[m_usedSize] = c;

	return m_usedSize++;
}

ByteArray ByteArray::asHex(uint8_t separator) const
{
	if(m_usedSize == 0)
		return ByteArray();

	ByteArray s;
	s.reserve(m_usedSize*3);

	for(size_t i=0;i<m_usedSize;++i)
	{
		ByteArray b(4);
		snprintf((char*)b.internalBuffer(), b.size(), "%02x", m_memory[i]);
		
		b.resize(b.size()-2);
		
		s += b;

		if(i != m_usedSize-1 && separator != 0)
			s.append(separator);
	}
	
	// To upper case
	for(size_t i=0;i<s.size();++i)
		if(s[i] >= 'a' && s[i] <= 'f')
			s[i] -= ('a' - 'A');

	return s;
}

int32_t ByteArray::asInt() const
{
	int32_t i;
	sscanf((const char*)internalBuffer(), "%ld", &i);
	return i;
}

#ifdef ALLOW_ARDUINO_STRINGS
String ByteArray::toPrintableHexString(char separator) const
{
	if(m_usedSize == 0)
		return String();

	String s;
	s.reserve(m_usedSize*3);

	for(size_t i=0;i<m_usedSize;++i)
	{
		char b[4] = {0};
		sprintf(b, "%02x", m_memory[i]);
		s.concat(b);

		if(i != m_usedSize-1 && separator != 0)
			s.concat(separator);
	}

	return s;
}
#endif
#ifdef ALLOW_ARDUINO_STREAMS
void ByteArray::printToStream(Stream& s)
{
	for (size_t i = 0; i<m_usedSize; ++i)
	{
		char b[4] = { 0 };
		sprintf(b, "%02x", m_memory[i]);
		s.print(b);

		if (i != m_usedSize - 1)
			s.print(':');
	}

	s.println("");
}

void ByteArray::writeToStream(Stream& s)
{
	if(m_usedSize)
		s.write(m_memory, m_usedSize);
}
#endif

ByteArray ByteArray::fromHex(const ByteArray& s)
{
	ByteArray b;

	int8_t c = -1;

	for(size_t i=0;i<s.size();++i)
	{
		int8_t val = -1;
		if(s[i] >= '0' && s[i] <= '9')
			val = s[i] - '0';
		else if(s[i] >= 'A' && s[i] <= 'F')
			val = s[i] - 'A' + 10;
		else if(s[i] >= 'a' && s[i] <= 'f')
			val = s[i] - 'a' + 10;

		if(val != -1)
		{
			if(c != -1)
			{
				b.append((c << 4) + val);
				c = -1;
			}
			else
				c = val;
		}
	}

	return b;
}
