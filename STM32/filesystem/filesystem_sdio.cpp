#include "filesystem_sdio.h"

Filesystem::Filesystem(const Pin& sdDetect, const Pin& writeProtect)
        : m_sdDetect(sdDetect), m_writeProtect(writeProtect)
{
    if(m_sdDetect)
        m_sdDetect.init(GPIO_MODE_INPUT, GPIO_PULLUP);

    if(m_writeProtect)
        m_writeProtect.init(GPIO_MODE_INPUT, GPIO_PULLUP);
}

Filesystem::~Filesystem()
{
    unmount();
}

FRESULT Filesystem::mount()
{
    if(!isSDCardPresent())
        return FR_DISK_ERR;

    return f_mount(&m_fs, "0:", 1);
}

FRESULT Filesystem::unmount()
{
    return f_mount(nullptr, "0:", 1);
}

bool Filesystem::isSDCardPresent() const
{  
    return m_sdDetect?(!m_sdDetect.read()):true;
}

bool Filesystem::isWriteProtected() const
{
    return m_writeProtect?(m_writeProtect.read()):false;
}
