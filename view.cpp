#include "view.h"
#include <QDebug>

view::view(QWidget *parent) : QGraphicsView(parent)
{
    emit signalViewInit();
    qDebug()<<"signal view init emited";
    //qDebug()<<"view width: "<< this->width()<<"view height: "<< this->height();
    this->setMouseTracking(true);
}
