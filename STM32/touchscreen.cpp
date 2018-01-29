#include "touchscreen.h"
#include "system.h"

TouchScreen::TouchScreen(Display* display, Touch* touch):
    m_display(display), m_touch(touch)
{}

TouchScreen::~TouchScreen()
{
    delete m_display;
    delete m_touch;
}

bool TouchScreen::init(Orientation orientation)
{
    if(!m_display || !m_touch)
        return true;

    display()->init(orientation);
    touch()  ->init(orientation);

    touch()->setCalibrationData(Point2D(21, 30), 1.12119, 1.354113);

    display()->fillScreen(Color16::Black);

    return false;
}

void TouchScreen::calibrate()
{
    Point2D s = m_display->size();

    Point2D ptsRef[] = {Point2D(10, 10), Point2D(s.x()-10, s.y()-10), Point2D(10, s.y()-10), Point2D(s.x()-10, 10), s/2};
    Point2D pts[5];

    m_touch->setCalibrationData(Point2D(0, 0), 1.f, 1.f);

    for(int i=0; i < 5; ++i)
    {
        m_display->fillScreen(Color16::Black);
        m_display->fillCircle(ptsRef[i].x(), ptsRef[i].y(), 3, Color16::White);

        System::delay(1000);

        std::pair<bool, Point2D> p;

        do
        {
            p = m_touch->update();
        } while(!p.first);

        pts[i] = p.second;
    }

    for(int i=0; i < 5; ++i)
        printf("x %03ld (%03ld, %03ld) \t| y %03ld (%03ld, %03ld)\r\n", pts[i].x(),  ptsRef[i].x(), pts[i].x() - ptsRef[i].x(),
                                                                        pts[i].y(),  ptsRef[i].y(), pts[i].y() - ptsRef[i].y());


    Point2D low( ((pts[0].x()-ptsRef[0].x())+pts[2].x()-ptsRef[2].x())/2,
                 ((pts[0].y()-ptsRef[0].y())+pts[3].y()-ptsRef[3].y())/2);

    for(int i=0; i < 5; ++i)
        pts[i] -= low;
    
    float sx = ((static_cast<float>(ptsRef[1].x())/static_cast<float>(pts[1].x())) + (static_cast<float>(ptsRef[3].x())/static_cast<float>(pts[3].x())))/2.f;
    float sy = ((static_cast<float>(ptsRef[1].y())/static_cast<float>(pts[1].y())) + (static_cast<float>(ptsRef[2].y())/static_cast<float>(pts[2].y())))/2.f;

    m_touch->setCalibrationData(low, sx, sy);

    printf("low: %03ld %03ld \t high: %f %0f\r\n", low.x(), low.y(), sx, sy);

}

Display* TouchScreen::display()
{
    return m_display;
}

Touch* TouchScreen::touch()
{
    return m_touch;
}