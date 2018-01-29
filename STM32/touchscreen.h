/**
    \file touchscreen.h
    \author Arnaud CADOT
    \version 1.0
**/
#ifndef GUARD_TOUCHSCREEN
#define GUARD_TOUCHSCREEN

#include <cstdint>
#include "point2d.h"
/*
#include "drivers/xpt2046/xpt2046.h"
#include "drivers/ili9341/ili9341.h"*/
#include "display.h"
#include "touch.h"

/**
    \brief Generic class to interface a display and its touchscreen driver.
**/
class TouchScreen
{
    public:
        /**
            \brief Constructor
            \param display Display to use
            \param touch Touch driver to use
            \remark The newly created instance will assume ownership of both pointers and will
            free the memory at the appropriate time.
        **/
        TouchScreen(Display* display, Touch* touch);

        /**
            \brief Destructor
        **/
        ~TouchScreen();
        
        /**
            \brief Initialize both display and touch driver
            \param orientation Orientation to use
            \return \c true on error, \c false otherwise
        **/
        bool init(Orientation orientation = Orientation::LANDSCAPE_2);

        /**
            \brief Start a (blocking) calibration routing
            \remark User interaction is required for calibration to complete
        **/
        void calibrate();

        /**
            \return Display instance
        **/
        Display* display();

        /**
            \return Touch driver instance
        **/
        Touch* touch();

    private:
        Display* m_display  = nullptr;
        Touch* m_touch    = nullptr;

};

#endif
