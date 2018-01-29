#include "touch.h"

Touch::Touch()
{
}

Touch::~Touch()
{
}

Orientation Touch::orientation() const
{
    return m_orientation;
}

void Touch::setOrientation(Orientation orientation)
{
    m_orientation = orientation;
}

void Touch::setCalibrationData(Point2D minCorner, float x, float y)
{   
    m_minCorner.x() = minCorner.x();
    m_minCorner.y() = minCorner.y();

    m_stretchX = x;
    m_stretchY = y;
}

void Touch::setSize(const Point2D& size)
{
    m_size = size;
}

const Point2D& Touch::size() const
{
    return m_size;
}
