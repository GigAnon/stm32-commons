/**
    \file point2d.h
    \author Arnaud CADOT
    \version 1.0
**/
#ifndef GUARD_POINT2D
#define GUARD_POINT2D

#include <cstdint>

/**
    \brief Represent one out of four orientations for screens, etc.
**/
enum class Orientation
{
    LANDSCAPE_1,    ///<
    LANDSCAPE_2,    ///<
    PORTRAIT_1,     ///<
    PORTRAIT_2      ///<
};

/**
    \brief Basic 2D vertex

    Store a basic 2D vertex or point. Coordinates are (x,y) and stored in
    signed 32 bits integers.
**/
class Point2D
{
    public:
        /**
            \brief Constructor
            \param x x coordinate
            \parma y y coordinate
        **/
        Point2D(int32_t x, int32_t y);

        /**
            \brief Default constructor
            \remark Coordinates are both set to 0.
            \remark (Point2D() == Point2D(0, 0)) is \c true
        **/
        Point2D();

        /**
            \brief Comparaison operator
            \param other Other instance to compare against
            \returns \c true if coordinates are equals, \c false otherwise
        **/
        bool operator==(const Point2D& other) const;

        /**
            \brief Inequality operator
            \param other Other instance to compare against
            \returns \c false if coordinates are equals, \c true otherwise
        **/
        bool operator!=(const Point2D& other) const;

        /**
            \brief Assignment operator
            \param other Other instance to copy into this instance
            \returns Reference to current instance
        **/
        Point2D& operator=(const Point2D& other);

        /**
            \brief Substraction operator
            \param other Other instance to be substracted from this instance
            \returns Result
        **/
        Point2D  operator-(const Point2D& other);

        /**
            \brief Addition operator
            \param other Other instance to be added to this instance
            \returns Result
        **/
        Point2D  operator+(const Point2D& other);

        /**
            \brief Self-substraction operator
            \param other Other instance to be substracted from this instance
            \returns Reference to current instance
        **/
        Point2D& operator-=(const Point2D& other);

        /**
            \brief Self-addition operator
            \param other Other instance to be added to this instance
            \returns Reference to current instance
        **/
        Point2D& operator+=(const Point2D& other);

        /**
            \brief Scalar multiplication
            \param a Scalar to multiply the current instance with
            \returns Result
        **/
        Point2D operator*(int32_t a);

        /**
            \brief Scalar division
            \param a Scalar to divide the current instance with
            \returns Result
        **/
        Point2D operator/(int32_t a);

        /**
            \brief Scalar self-multiplication
            \param a Scalar to multiply the current instance with
            \returns Reference to current instance
        **/
        Point2D& operator*=(int32_t a);
        
        /**
            \brief Scalar self-division
            \param a Scalar to divide the current instance with
            \returns Reference to current instance
        **/
        Point2D& operator/=(int32_t a);

        /**
            \brief Access to x coordinate via reference
            \returns Reference to x coordinate
        **/
        int32_t& x();

        /**
            \brief Const access to x coordinate via copy
            \returns x coordinate
        **/
        int32_t  x() const;

        /**
            \brief Access to y coordinate via reference
            \returns Reference to y coordinate
        **/
        int32_t& y();

        /**
            \brief Const access to y coordinate via copy
            \returns y coordinate
        **/
        int32_t  y() const;

        /**
            \brief Set coordinates
            \param x x coordinate
            \param y y coordinate
            \returns Reference to current instance
        **/
        Point2D& set(int32_t x, int32_t y);

    private:
        int32_t m_x;
        int32_t m_y;
};

/**
    \brief Negation operator
    \param other Point2D to negate
    \return Result
**/
Point2D operator-(const Point2D& other);

#endif
