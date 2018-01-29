/**
    \file filesystem_sdio.h
    \author Arnaud CADOT
    \version 1.0
**/
#ifndef GUARD_FILESYSTEM_SDIO
#define GUARD_FILESYSTEM_SDIO

extern "C"
{
    #include "fatfs/ff.h"
}

#include "pin.h"

/*
    \brief FatFS-based overlay for SD card management

    \see FatFS documentation: http://elm-chan.org/fsw/ff/00index_e.html
*/
class Filesystem
{
    public:
        /*
            \brief Constructor
            \param sdDetect SD detection pin. Use Pin() if unused.
            \param writeProtect Write protection pin. Use Pin() if unused.
        */
        Filesystem(const Pin& sdDetect = Pin(), const Pin& writeProtect = Pin());

        /*
            No copy.
        */
        Filesystem(const Filesystem&) = delete;

        /*
            Destructor.
        */
        ~Filesystem();

        /*
            \brief Mount SD card filesystem.
            \return Error code. See FatFS documentation for details.
        */
        FRESULT mount();

        /*
            \brief Unmount SD card filesystem.
            \return Error code. See FatFS documentation for details.
        */
        FRESULT unmount();

        /*
            \returns \c true if a SD card was detected, \c false otherwise
            \remark When the SD detect feature is disabled, this method always returns \c true.
        */
        bool isSDCardPresent() const;

        /*
            \returns \c true the SD card is write protected, \c false otherwise
            \remark When the SD write protection feature is disabled, this method always returns \c false.
        */
        bool isWriteProtected() const;

    private:

        FATFS   m_fs = {};
        Pin     m_sdDetect;
        Pin     m_writeProtect;
};

#endif
