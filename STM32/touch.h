/**
    \file touch.h
    \author Arnaud CADOT
    \version 1.0
**/
#ifndef GUARD_TOUCH
#define GUARD_TOUCH

#include "point2d.h"
#include <utility>

/**
    \brief Abstract virtual base class for the 'touch' part of touch screens drivers

    Abstract base class for the 'touch' part of touch screens drivers. Daughter classes must
    implement init() and update() methods.
**/
class Touch
{
    public:
        /**
            Constructor
        **/
        Touch();

        /**
            Destructor
        **/
        virtual ~Touch();
        
        /**
            \brief Initialize the driver
            \param orientation Screen orientation
            \returns \c true on error, \c false otherwise

            \remark Must be implemented by daughter classes
        **/
        virtual bool init(Orientation orientation) = 0;

        /**
            \brief Update method. Must be called at regular intervals
            \returns Pair: (pressed, point), where \c pressed is \c true if the screen is being
            touched, in which case \c point are the coordinates of the press

            \remark Must be implemented by daughter classes
        **/
        virtual std::pair<bool, Point2D> update() = 0;

        /**
            \returns Screen orientation
        **/
        Orientation orientation() const;

        /**
            \brief Set screen orientation
            \param orientation New orientation
        **/
        void setOrientation(Orientation orientation);

        /**
            \brief Set screen size
            \param size New screen size
        **/
        void setSize(const Point2D& size);

        /**
            \returns Screen size
        **/
        const Point2D& size() const;

        /**
            \brief Set the calibration data for the touch screen
            \param minCorner Real upper-left corner position
            \param stretchX Linear stretch factor for X axis
            \param stretchY Linear stretch factor for Y axis
        **/
        void setCalibrationData(Point2D minCorner, float stretchX, float stretchY);

    protected:
        Orientation m_orientation = Orientation::LANDSCAPE_2;

        Point2D     m_size;

        Point2D     m_minCorner;
        float       m_stretchX;
        float       m_stretchY;
};

#endif
