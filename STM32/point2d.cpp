#include "point2d.h"

Point2D::Point2D(int32_t _x, int32_t _y):
    m_x(_x), m_y(_y)
{}

Point2D::Point2D(): Point2D(0, 0) {}

bool Point2D::operator==(const Point2D& other) const
{
    return  other.x() == x() &&
            other.y() == y();
}

bool Point2D::operator!=(const Point2D& other) const
{
    return !((*this) == other);
}

Point2D& Point2D::operator=(const Point2D& other)
{
    return set(other.x(), other.y());
}

Point2D Point2D::operator-(const Point2D& other)
{
    return (*this) + (-other);
}

Point2D& Point2D::operator-=(const Point2D& other)
{
    return (*this)=(*this)-other;
}

Point2D& Point2D::operator+=(const Point2D& other)
{
    return (*this)=(*this)+other;
}


Point2D Point2D::operator+(const Point2D& other)
{
    return Point2D(x() + other.x(), y() + other.y());
}

Point2D Point2D::operator*(int32_t a)
{
    return Point2D(a*x(), a*y());
}

Point2D Point2D::operator/(int32_t a)
{
    return Point2D(x()/a, y()/a);
}

Point2D& Point2D::operator*=(int32_t a)
{
    return (*this) = (*(this)*a);
}

Point2D& Point2D::operator/=(int32_t a)
{
    return (*this) = (*(this)/a);
}

int32_t& Point2D::x()
{
    return m_x;
}

int32_t Point2D::x() const
{
    return m_x;
}

int32_t& Point2D::y()
{
    return m_y;
}

int32_t Point2D::y() const
{
    return m_y;
}

Point2D& Point2D::set(int32_t _x, int32_t _y)
{
    x() = _x;
    y() = _y;
    return *this;
}

Point2D operator-(const Point2D& other)
{
    return Point2D(-other.x(), -other.y());
}
