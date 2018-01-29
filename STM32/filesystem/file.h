/**
    \file file.h
    \author Arnaud CADOT
    \version 1.0
**/
#ifndef GUARD_FILE
#define GUARD_FILE

/* IMPORTANT NOTE FOR STM32F4 HAL USERS */
/* There's a bug in HAL when writing to SD cards, see: */
/* https://community.st.com/thread/41225-buffer-overflow-in-cubemx-sdmmc-driver */
/* The proposed patch doesn't work. Use this patch instead: */
/* 
In HAL_SD_WriteBlocks (stm32f4xx_hal_sd.c) replace:

tempbuff += 8U;

With:

if( (tempbuff - (uint32_t *)pData) < (NumberOfBlocks * BLOCKSIZE / sizeof(uint32_t) - 8U) )
    tempbuff += 8U;
else
    break;
*/

extern "C"
{
    #include "fatfs/ff.h"
}

#include "byte_array.h"
#include <cstdint>

/*
    \brief Basic overlay for files on a SD card filesystem

    Basic overlay for files on a SD card filesystem, based on FatFS. 
    To use this class, a valid filesystem must have been already mounted.

    \see FatFS documentation: http://elm-chan.org/fsw/ff/00index_e.html
*/
class File
{
    public:
        /*
            File opening mode
            \remark See FatFS documentation for more details
        */
        struct Mode
        {
            enum Value
            {
                READ            = FA_READ,
                WRITE           = FA_WRITE,
                OPEN_EXISTING   = FA_OPEN_EXISTING,
                CREATE_NEW      = FA_CREATE_NEW,
                OVERWRITE       = FA_CREATE_ALWAYS,
                APPEND          = FA_OPEN_APPEND,

                /* POSIX file flags */
                R       = READ,
                R_PLUS  = READ | WRITE,
                W       = OVERWRITE | WRITE,
                W_PLUS  = W | READ,
                A       = APPEND | WRITE,
                A_PLUS  = A | READ,
                X       = CREATE_NEW | WRITE,
                X_PLUS  = X | READ
            };
        };

    public:
        /**
            Constructor.
        **/
        File();

        /**
            No copy.
        **/
        File(const File&) = delete;

        /**
            Destructor.
        **/
        ~File();

        /**
            \brief Open a file
            \param filename Path to the file to be opened
            \param mode Opening mode. \see File::Mode
            \return Error code. See FatFS documentation
        **/
        FRESULT open(const char* const filename, Mode::Value mode = Mode::R_PLUS);

        /**
            \brief Close the file
            \return Error code. See FatFS documentation
        **/
        FRESULT close();

        /**
            \brief Read data from the file
            \param data Buffer the data will be written to
            \param maxData Maximum bytes to be read
            \returns Bytes actually read from the file. Returns 0 on errors
        **/
        uint32_t read(uint8_t* data, uint32_t maxData);

        /**
            \brief Read data from the file
            \param data Buffer the data will be written to
            \param maxData Maximum bytes to be read
            \returns Error code. See FatFS documentation
        **/
        FRESULT read(ByteArray& data, uint32_t maxData = 0xFFFFFFFF);

        /**
            \brief Write data to the file
            \param data Data to be written
            \returns Error code. See FatFS documentation
        **/
        FRESULT write(const ByteArray& data);

        /**
            \returns Position of the virtual file cursor
        **/
        FSIZE_t tell() const;

        /**
            \returns File size
        **/
        FSIZE_t size() const;

        /**
            \brief Set the virtual cursor at the beginning of the file
            \returns Error code. See FatFS documentation
        **/
        FRESULT rewind();

        /**
            \brief Set the position of the virtual cursor
            \param p New cursor position
            \returns Error code. See FatFS documenation

            \remark Unlike FatFS function f_lseek(), past-the-end cursors positions
            are considered invalid and the method will return a FR_INVALID_PARAMETER error.
        **/
        FRESULT seek(FSIZE_t p);

        bool isOpen() const { return m_open; }

    private:
        FIL m_file;
        bool m_open = false;
};

#endif
