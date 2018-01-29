#include "file.h"

#include <algorithm>

File::File()
{
}

File::~File()
{
    close();
}

FRESULT File::open(const char* const filename, Mode::Value mode)
{
    if(isOpen())
        close();

    FRESULT r = f_open(&m_file, filename, mode);
    
    if(r == FR_OK)
        m_open = true;

    return r;
}

FRESULT File::close()
{
    return f_close(&m_file);
}

uint32_t File::read(uint8_t* data, uint32_t maxData)
{
    UINT d = 0;

    if(f_read(&m_file, reinterpret_cast<void*>(data), maxData, &d) != FR_OK)
        return 0;

    return d;
}

FRESULT File::read(ByteArray& data, uint32_t maxData)
{
    data.clear();

    if(maxData == 0)
        return FR_OK;

    data.resize(std::min((FSIZE_t)maxData, size() - tell()));

    UINT d = 0;

    if(auto r = f_read(&m_file, reinterpret_cast<void*>(data.internalBuffer()), data.size(), &d))
    {
        data.clear();
        return r;
    }

    data.resize(d);
    data.shrink();
    
    return FR_OK;
}

FRESULT File::write(const ByteArray& data)
{
    UINT d;
    return f_write(&m_file, reinterpret_cast<const void*>(data.internalBuffer()), data.size(), &d);
}

FRESULT File::rewind()
{
    return seek(0);
}

FRESULT File::seek(FSIZE_t p)
{
    if(p >= size())
        return FR_INVALID_PARAMETER;

    return f_lseek(&m_file, p);
}

FSIZE_t File::tell() const
{
    return f_tell(const_cast<FIL*>(&m_file));
}

FSIZE_t File::size() const
{
    return f_size(const_cast<FIL*>(&m_file));
}
