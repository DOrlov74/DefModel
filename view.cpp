#include "view.h"
#include <QDebug>
#include <QWheelEvent>

view::view(QWidget *parent) : QGraphicsView(parent)
{
    emit signalViewInit();
    qDebug()<<"signal view init emited";
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
