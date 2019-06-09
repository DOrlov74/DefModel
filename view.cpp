#include "view.h"
#include <QDebug>
#include <QWheelEvent>

view::view(QWidget *parent) : QGraphicsView(parent)
{
    //emit signalViewInit();
    //qDebug()<<"signal view init emited";
    //qDebug()<<"view width: "<< this->width()<<"view height: "<< this->height();
    setMouseTracking(true);
}

void view::slotPan(bool b)
{
    qDebug()<<"in Pan Slot";
    m_panMode=b;
    if (b)
    {setDragMode(QGraphicsView::ScrollHandDrag);}
    else
    {setDragMode(QGraphicsView::NoDrag);}
}


void view::wheelEvent(QWheelEvent *event)
{
    if(event->delta() > 0)
          scale(1.25, 1.25);
        else
          scale(0.8, 0.8);
}


void view::slotZoomIn()
{
    this->scale(1.25,1.25);
}

void view::slotZoomOut()
{
    this->scale(0.8,0.8);
}


void view::mousePressEvent(QMouseEvent *event)
{
    if (m_panMode)
    {
        m_currPoint=event->screenPos();
    }
    emit QGraphicsView::mousePressEvent(event);
}

void view::mouseMoveEvent(QMouseEvent *event)
{
    if (m_panMode)
    {
        translate(event->screenPos().x()-m_currPoint.x(), event->screenPos().y()-m_currPoint.y());
        m_currPoint=event->screenPos();
    }
    emit QGraphicsView::mouseMoveEvent(event);
}
