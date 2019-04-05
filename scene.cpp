#include "scene.h"
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QPointF>
#include <QPainterPathStroker>
#include <QCursor>
#include <QGraphicsItem>
#include <QGraphicsEllipseItem>
#include <QAbstractGraphicsShapeItem>
#include <QRegularExpression>
#include <QString>


Scene::Scene(QWidget *parent) : QGraphicsScene(parent)
{
    m_concretePath=new QPainterPath();
    m_concretePath->setFillRule(Qt::WindingFill);
    //QObject::connect(this, SIGNAL(signalSceneInit()), this, SLOT(slotSceneInit()), Qt::QueuedConnection);
    //idTimer=startTimer(50);
}

Scene::drawMode Scene::getDrawMode()
{
    return m_drawMode;
}

void Scene::setBasePoint(QPointF point)
{
    m_basePoint=point;
    qDebug()<<"scene base point: "<<m_basePoint;
}

void Scene::setDrawLine()
{
    m_drawMode=LINE;
}

void Scene::setDrawRect()
{
    m_drawMode=RECT;
}

void Scene::slotGetCommand(QString str)
{
    qDebug()<<"command: "<<str;
    if (m_drawMode==LINE&&(m_doneConcretePath==false)&&(m_concretePoints.size()>1))
    {
        if (str=="c"||str=="C")
        {
            m_concretePath->closeSubpath();
            m_drawMode=NONE;
            m_doneConcretePath=true;
            qDebug()<<"draw mode is off";
            qDebug()<<"selected items: "<<m_pointsItems.size();
            for (QGraphicsItem* item: m_pointsItems)
            {
                this->removeItem(item);
            }
            m_pointsItems.clear();
            //this->destroyItemGroup(m_pointsGroup);
            this->addPath(*m_concretePath, pen, brush);
            emit signalDrawMode(false);
        }
    }
    if (m_drawMode!=NONE&&(m_doneConcretePath==false))
    {
        QPointF p;
        QRegularExpression reg("[+-]?([0-9]*[.])?[0-9]+");
        QRegularExpressionMatchIterator it=reg.globalMatch(str);
        while (it.hasNext())
        {
            QRegularExpressionMatch match=it.next();
            qDebug()<<"filtered coordinats: x="<<match.captured(0);
            p.setX(QString(match.captured(0)).toDouble());
            match=it.next();
            qDebug()<<"filtered coordinats: y="<<match.captured(0);
            p.setY(QString(match.captured(0)).toDouble());
        }
         drawPoint(fromSceneCoord(p));
    }
}

void Scene::slotSceneInit()
{
    m_basePoint=QPointF(10,this->height()-10);
    qDebug()<<"scene base point: "<<m_basePoint;
}

void Scene::slotDivide()
{
    double lowX=m_concretePoints[0].x();
    double highX=m_concretePoints[0].x();
    double lowY=m_concretePoints[0].y();
    double highY=m_concretePoints[0].y();
    for (QPointF p: m_concretePoints)
    {
        if (p.x()<lowX)
        {lowX=p.x();}
        else if (p.x()>highX)
        {highX=p.x();}
        if (p.y()<lowY)
        {lowY=p.y();}
        else if (p.y()>highY)
        {highY=p.y();}
    }
    m_recWidth=highX-lowX;
    m_recHeight=highY-lowY;
}

QPointF Scene::toSceneCoord(const QPointF& point)
{
    return QPointF(point.x()-m_basePoint.x(),-point.y()-m_basePoint.y());
}

QPointF Scene::fromSceneCoord(const QPointF& point)
{
    return QPointF(point.x()+m_basePoint.x(),-point.y()-m_basePoint.y());
}

void Scene::drawPoint(const QPointF& point)
{
    m_concretePoints.append(point);

    if (m_concretePath->currentPosition()==QPointF(0,0))
    {
        //this is the first node in the path
        m_concretePath-> moveTo(point);
        qDebug()<<"Path move to: "<<m_concretePath->currentPosition();

    }
    else
    {
        if (m_drawMode==LINE)
        {
            //this is not the first node in the path and we are drawing the line
            m_concretePath->lineTo(point);
            qDebug()<<"Path line to: "<<m_concretePath->currentPosition();
        }
        else
        {
            //we are drawing the rectangle
            m_concretePath->addRect(m_concretePoints[0].x(), m_concretePoints[0].y(),point.x()-m_concretePoints[0].x(),point.y()-m_concretePoints[0].y());
            qDebug()<<"Rect to: "<<m_concretePath->currentPosition();
            m_concretePoints[1]=QPointF(point.x(),m_concretePoints[0].y());
            pen.setBrush(QBrush(Qt::red));
            //m_currentItem=this->addEllipse(m_concretePoints[1].x()-m_pointSize/2, m_concretePoints[1].y()-m_pointSize/2,m_pointSize,m_pointSize,pen);
            //m_pointsItems.append(m_currentItem);
            m_concretePoints.append(point);
            m_concretePoints.append(QPointF(m_concretePoints[0].x(),point.y()));
            //m_currentItem=this->addEllipse(m_concretePoints[3].x()-m_pointSize/2, m_concretePoints[3].y()-m_pointSize/2,m_pointSize,m_pointSize,pen);
            //m_pointsItems.append(m_currentItem);
            m_drawMode=NONE;
            m_doneConcretePath=true;
            qDebug()<<"draw mode is off";
            //remove points from scene because the rectangle is finished
            for (QGraphicsItem* item: m_pointsItems)
            {
                this->removeItem(item);
            }
            m_pointsItems.clear();
            emit signalDrawMode(false);
        }
        pen.setBrush(Qt::black);
        // pen.setWidth(1);
        brush.setColor(Qt::yellow);
        this->addPath(*m_concretePath, pen, brush);
    }
    if (m_drawMode!=NONE){
        //we are drawing a point
    pen.setBrush(QBrush(Qt::red));
    m_currentItem=this->addEllipse(point.x()-m_pointSize/2, point.y()-m_pointSize/2,m_pointSize,m_pointSize,pen);
    m_pointsItems.append(m_currentItem);
    //m_pointsGroup->addToGroup(m_currentItem);
    }
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
    {
        if (m_drawMode!=NONE&&(m_doneConcretePath==false))
        {
            //drawMode is on and concrete path is not closed yet
            qDebug()<<"Mouse at: "<<event->scenePos();
            drawPoint(event->scenePos());
            emit signalPointAdded(toSceneCoord(event->scenePos()));
        }
    }
    QGraphicsScene::mousePressEvent(event);
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_drawMode)
    {

    }
}

void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
}


void Scene::timerEvent(QTimerEvent *event)
{
    emit signalSceneInit();
    killTimer(idTimer);
}
