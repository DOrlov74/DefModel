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
#include <QtMath>
#include <cmath>

Scene::Scene(QWidget *parent) : QGraphicsScene(parent)
{
    m_concretePath=new QPainterPath();
    m_concretePath->setFillRule(Qt::WindingFill);
    //QObject::connect(this, SIGNAL(signalSceneInit()), this, SLOT(slotSceneInit()), Qt::QueuedConnection);
    //idTimer=startTimer(50);
    //emit signalGetRDiameter(m_currDiam);
    //qDebug()<<"Current diameter:"+QString::number(m_currDiam);
    myCalc=new Calculation;
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

int Scene::getCurrDiam()
{
    return m_currDiam;
}

void Scene::slotSetRDiameter(int i)
{
    m_currDiam=i;
    qDebug()<<"Current diameter:"+QString::number(m_currDiam);
}

void Scene::slotNewReinf()
{
    qDebug()<<"in new reinforcement method";
    int i=0;
    for (QGraphicsEllipseItem* item: m_reinfItems)
    {this->removeItem(item);
    qDebug()<<"reinforcement item " + QString::number(i) + " removed";
    delete item;
    ++i;
    }
    m_reinfCircles.erase(m_reinfCircles.begin(),m_reinfCircles.end());
    m_doneReinforcement=false;
    emit signalReinfCleared(true);
}

void Scene::slotCalculate()
{
    qDebug()<<"in Calculate Slot";
    myCalc->setXdivision(nXdivisions);
    myCalc->setYdivision(nYdivisions);
    m_concreteArea.fill(QVector<double>(),nXdivisions);
    m_concreteCenter.fill(QVector<QPointF>(),nXdivisions);
    if (m_isRect)
    {
        double dWidth;
        double dHeight;
        for (int i=1; i<m_dividedPoints.size(); ++i)
        {
            m_concreteArea[i-1].fill(0,nYdivisions);
            m_concreteCenter[i-1].fill(QPointF(0,0),nYdivisions);
            for (int j=1; j<m_dividedPoints[i].size();++j)
            {
                dWidth=qSqrt(qPow((m_dividedPoints[i][j-1].x()-m_dividedPoints[i-1][j-1].x()),2)+qPow((m_dividedPoints[i][j-1].y()-m_dividedPoints[i-1][j-1].y()),2));
                dHeight=qSqrt(qPow((m_dividedPoints[i-1][j].x()-m_dividedPoints[i-1][j-1].x()),2)+qPow((m_dividedPoints[i-1][j].y()-m_dividedPoints[i-1][j-1].y()),2));
                m_concreteArea[i-1][j-1]=dWidth*dHeight;
                m_concreteCenter[i-1][j-1].setX((m_dividedPoints[i-1][j-1].x()+m_dividedPoints[i][j-1].x()+m_dividedPoints[i-1][j].x()+m_dividedPoints[i][j].x())/4);
                m_concreteCenter[i-1][j-1].setY((m_dividedPoints[i-1][j-1].y()+m_dividedPoints[i][j-1].y()+m_dividedPoints[i-1][j].y()+m_dividedPoints[i][j].y())/4);
            }
        }
    }
    else
    {
        double dSum1=0;
        double dSum2=0;
        double dSumX=0;
        double dSumY=0;
        int iSize;
        for (int i=0; i<m_dividedRegions.size(); ++i)
        {
            m_concreteArea[i].fill(0,nYdivisions);
            m_concreteCenter[i].fill(QPointF(0,0),nYdivisions);
            for (int j=0; j<m_dividedRegions[i].size();++j)
            {
                iSize=m_dividedRegions[i][j].size();
                for (int k=0; k<iSize-1;++k)
                {
                    dSum1+=m_dividedRegions[i][j][k].x()*m_dividedRegions[i][j][k+1].y();
                    dSum2+=m_dividedRegions[i][j][k+1].x()*m_dividedRegions[i][j][k].y();
                    dSumX+=m_dividedRegions[i][j][k].x();
                    dSumY+=m_dividedRegions[i][j][k].y();
                }
                if (iSize!=0)
                {
                    m_concreteArea[i][j]=0.5*abs(dSum1+m_dividedRegions[i][j][iSize-1].x()*m_dividedRegions[i][j][0].y()-dSum2-m_dividedRegions[i][j][0].x()*m_dividedRegions[i][j][iSize-1].y());
                    m_concreteCenter[i][j].setX(dSumX/iSize);
                    m_concreteCenter[i][j].setY(dSumY/iSize);
                    dSum1=0;
                    dSum2=0;
                    dSumX=0;
                    dSumY=0;
                }
                else
                {
                    m_concreteArea[i][j]=0;
                }

            }
        }
    }
    myCalc->setConcreteArea(m_concreteArea);
    myCalc->setConcreteCenter(m_concreteCenter);
    myCalc->setReinfArea(m_reinfCircles);
    myCalc->setCenterPoint();
    myCalc->setMomentsOfInertia();
}

void Scene::slotFitView()
{
    this->setSceneRect(lowX-m_viewMargin,lowY-m_viewMargin,m_recWidth+m_viewMargin, m_recHeight+m_viewMargin);
    //emit signalFitView();
}

void Scene::setDrawLine()
{
    m_drawMode=LINE;
    emit signalDrawMode(true);
}

void Scene::setDrawRect()
{
    m_drawMode=RECT;
    emit signalDrawMode(true);
}

void Scene::setDrawPoint()
{
    m_drawMode=POINT;
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
            m_isRect=false;
//            double a1, a2, angle;
//            int pSize=m_concretePoints.size()-1;
//            if (m_concretePoints[0].x()==m_concretePoints[1].x())   //first line and X axis are parallel
//            {
//                if(m_concretePoints[0].y()<m_concretePoints[1].y())
//                {a1=M_PI_2;}
//                else
//                {a1=M_PI+M_PI_2;}
//            }
//            else if(m_concretePoints[0].x()<m_concretePoints[1].x())    //first line is in positive half of X axis
//            {
//                a1=qAtan((m_concretePoints[1].y()-m_concretePoints[0].y())/(m_concretePoints[1].x()-m_concretePoints[0].x()));
//            }
//            else {                                                      //first line is in negative half of X axis
//                a1=M_PI+qAtan((m_concretePoints[1].y()-m_concretePoints[0].y())/(m_concretePoints[1].x()-m_concretePoints[0].x()));
//            }
//            if(a1<0){a1=2*M_PI+a1;}
//            if (m_concretePoints[0].x()==m_concretePoints[pSize].x())   //last line and X axis are parallel
//            {
//                if(m_concretePoints[0].y()<m_concretePoints[pSize].y())
//                {a2=M_PI_2;}
//                else
//                {a2=M_PI+M_PI_2;}
//            }
//            else if(m_concretePoints[0].x()<m_concretePoints[pSize].x())    //last line is in positive half of X axis
//            {
//                a2=qAtan((m_concretePoints[pSize].y()-m_concretePoints[0].y())/(m_concretePoints[pSize].x()-m_concretePoints[0].x()));
//            }
//            else {                                                          //last line is in negative half of X axis
//                a2=M_PI+qAtan((m_concretePoints[pSize].y()-m_concretePoints[0].y())/(m_concretePoints[pSize].x()-m_concretePoints[0].x()));
//            }
//            if(a2<0){a2=2*M_PI+a2;}
//            angle=a2-a1;
//            if(angle<M_PI)
//            {
//                m_leftToRight=true;
//            }
//            else {m_leftToRight=false;}
//            qDebug()<<"angle 0 - 1: "+QString::number(a1)+"angle 0 - last: "+QString::number(a2);
//            qDebug()<<(m_leftToRight?"from left to right":"from right to left");
            qDebug()<<"draw mode is off";
            qDebug()<<"selected items: "<<m_pointsItems.size();
            for (QGraphicsItem* item: m_pointsItems)
            {
                this->removeItem(item);
            }
            m_pointsItems.clear();
            //this->destroyItemGroup(m_pointsGroup);
            pen.setBrush(Qt::black);
            // pen.setWidth(1);
            brush.setColor(Qt::yellow);
            this->addPath(*m_concretePath, pen, brush);
            emit signalDrawMode(false);
            emit signalSectDone(true);
            getSectSizes();
        }
    }
    if (m_drawMode==POINT)
    {
        if (str=="d"||str=="D")
        {
            m_drawMode=NONE;
            m_doneReinforcement=true;
            emit signalReinfDone(true);
        }
    }
    if (m_drawMode!=NONE&&(m_doneConcretePath==false||m_doneReinforcement==false))
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

void Scene::Divide()
{
    qDebug()<<"in slot divide";
    m_dividedPoints.fill(QVector<QPointF>(),nXdivisions+1);
    if (m_isRect)       //simple divide
    {
        for (int i=0; i<=nXdivisions; ++i)
        {
            for (int j=0; j<=nYdivisions; ++j)
            {
                m_dividedPoints[i].append(QPointF(lowX+i*m_recWidth/nXdivisions,lowY+j*m_recHeight/nYdivisions));
            }
        }
    }
    else                //divide custom polygon
    {
        QVector<QVector<QPointF>> vIntersectX;  //vector to store points of intersection with X axis of divisions
        vIntersectX.fill(QVector<QPointF>(),2);
        vIntersectX[0].fill(QPointF(0,0),nXdivisions+1);
        vIntersectX[1].fill(QPointF(0,0),nXdivisions+1);
        double x, y, x1, x2, y1, y2;
        for (int i=0; i<nXdivisions+1; ++i)
        {
            x=lowX+i*m_recWidth/nXdivisions;
            qDebug()<<"curent x:"+QString::number(x);
            for (int j=1; j<m_concretePoints.size(); ++j)
            {
                x1=m_concretePoints[j].x();
                x2=m_concretePoints[j-1].x();
                y1=m_concretePoints[j].y();
                y2=m_concretePoints[j-1].y();
                if((x<=x1&&x>=x2)||(x>=x1&&x<=x2))
                {
                   if(x==x1){y=y1;}
                   else if(x==x2){y=y2;}
                   else
                   {
                       y=(y1-y2)/(x1-x2)*x+(x1*y2-x2*y1)/(x1-x2);
                    }
                    qDebug()<<"intersection is at y:"+QString::number(y);
                    if (vIntersectX[0][i].x()==0&&vIntersectX[0][i].y()==0)
                    {
                        vIntersectX[0][i]=QPointF(x,y);
                        qDebug()<<"Point added in line 1";
                    }
                    else
                    {
                        vIntersectX[1][i]=QPointF(x,y);
                        qDebug()<<"Point added in line 2";
                    }
                }
            }
            x1=m_concretePoints[m_concretePoints.size()-1].x();
            x2=m_concretePoints[0].x();
            y1=m_concretePoints[m_concretePoints.size()-1].y();
            y2=m_concretePoints[0].y();
            if((x<=x1&&x>=x2)||(x>=x1&&x<=x2))
            {
               if(x==x1){y=y1;}
               else if(x==x2){y=y2;}
               else
               {
                   y=(y1-y2)/(x1-x2)*x+(x1*y2-x2*y1)/(x1-x2);
                }
                qDebug()<<"last intersection is at y:"+QString::number(y);
                if (vIntersectX[0][i].x()==0&&vIntersectX[0][i].y()==0)
                {
                    vIntersectX[0][i]=QPointF(x,y);
                    qDebug()<<"Point added in line 1";
                }
                else
                {
                    vIntersectX[1][i]=QPointF(x,y);
                    qDebug()<<"Point added in line 2";
                }
            }
        }
//        qDebug()<<"Intersections line 1:\n";
//        for (int i=0; i<vIntersectX[0].size(); ++i)
//        {
//            qDebug()<<"x:"+QString::number(vIntersectX[0][i].x());
//            qDebug()<<" y:"+QString::number(vIntersectX[0][i].y());
//        }
//        qDebug()<<"Intersections line 2:\n";
//        for (int i=0; i<vIntersectX[1].size(); ++i)
//        {
//            qDebug()<<"x:"+QString::number(vIntersectX[1][i].x());
//            qDebug()<<" y:"+QString::number(vIntersectX[1][i].y());
//        }
        QVector<QVector<QPointF>> vIntersectY;  //vector to store points of intersection with Y axis of divisions
        vIntersectY.fill(QVector<QPointF>(),2);
        vIntersectY[0].fill(QPointF(0,0),nYdivisions+1);
        vIntersectY[1].fill(QPointF(0,0),nYdivisions+1);
        for (int i=0; i<nYdivisions+1; ++i)
        {
            y=lowY+i*m_recHeight/nYdivisions;
            qDebug()<<"curent y:"+QString::number(y);
            for (int j=1; j<m_concretePoints.size(); ++j)
            {
                x1=m_concretePoints[j].x();
                x2=m_concretePoints[j-1].x();
                y1=m_concretePoints[j].y();
                y2=m_concretePoints[j-1].y();
                if((y<=y1&&y>=y2)||(y>=y1&&y<=y2))
                {
                   if(y==y1){x=x1;}
                   else if(y==y2){x=x2;}
                   else
                   {
                       x=(x1-x2)/(y1-y2)*y+(y1*x2-y2*x1)/(y1-y2);
                    }
                    qDebug()<<"intersection is at x:"+QString::number(x);
                    if (vIntersectY[0][i].x()==0&&vIntersectY[0][i].y()==0)
                    {
                        vIntersectY[0][i]=QPointF(x,y);
                        qDebug()<<"Point added in line 1";
                    }
                    else
                    {
                        vIntersectY[1][i]=QPointF(x,y);
                        qDebug()<<"Point added in line 2";
                    }
                }
            }
            x1=m_concretePoints[m_concretePoints.size()-1].x();
            x2=m_concretePoints[0].x();
            y1=m_concretePoints[m_concretePoints.size()-1].y();
            y2=m_concretePoints[0].y();
            if((y<=y1&&y>=y2)||(y>=y1&&y<=y2))
            {
               if(y==y1){x=x1;}
               else if(y==y2){x=x2;}
               else
               {
                   x=(x1-x2)/(y1-y2)*y+(y1*x2-y2*x1)/(y1-y2);
                }
                qDebug()<<"last intersection is at x:"+QString::number(x);
                if (vIntersectY[0][i].x()==0&&vIntersectY[0][i].y()==0)
                {
                    vIntersectY[0][i]=QPointF(x,y);
                    qDebug()<<"Point added in line 1";
                }
                else
                {
                    vIntersectY[1][i]=QPointF(x,y);
                    qDebug()<<"Point added in line 2";
                }
            }
        }
        QPointF lowInterX1,highInterX1, lowInterY1, highInterY1;
        QPointF lowInterX2,highInterX2, lowInterY2, highInterY2;
        m_dividedRegions.fill(QVector<QVector<QPointF>>(),nXdivisions);
        for (int i=1; i<=nXdivisions; ++i)
        {
            m_dividedRegions[i-1].fill(QVector<QPointF>(),nYdivisions);
            for (int j=1; j<=nYdivisions; ++j)
            {
                x1=lowX+(i-1)*m_recWidth/nXdivisions;
                y1=lowY+(j-1)*m_recHeight/nYdivisions;
                x2=lowX+i*m_recWidth/nXdivisions;
                y2=lowY+j*m_recHeight/nYdivisions;
                if(vIntersectX[0][i-1].y()<vIntersectX[1][i-1].y())
                {
                    lowInterX1=vIntersectX[0][i-1];
                    highInterX1=vIntersectX[1][i-1];
                }
                else
                {
                    lowInterX1=vIntersectX[1][i-1];
                    highInterX1=vIntersectX[0][i-1];
                }
                if(vIntersectX[0][i].y()<vIntersectX[1][i].y())
                {
                    lowInterX2=vIntersectX[0][i];
                    highInterX2=vIntersectX[1][i];
                }
                else
                {
                    lowInterX2=vIntersectX[1][i];
                    highInterX2=vIntersectX[0][i];
                }
                if(vIntersectY[0][j-1].x()<vIntersectY[1][j-1].x())
                {
                    lowInterY1=vIntersectY[0][j-1];
                    highInterY1=vIntersectY[1][j-1];
                }
                else
                {
                    lowInterY1=vIntersectY[1][j-1];
                    highInterY1=vIntersectY[0][j-1];
                }
                if(vIntersectY[0][j].x()<vIntersectY[1][j].x())
                {
                    lowInterY2=vIntersectY[0][j];
                    highInterY2=vIntersectY[1][j];
                }
                else
                {
                    lowInterY2=vIntersectY[1][j];
                    highInterY2=vIntersectY[0][j];
                }
                if(lowInterX1.y()>=y1&&lowInterX1.y()<=y2)                                        //low intersection is on the line with x=x1
                {m_dividedRegions[i-1][j-1].append(QPointF(x1,lowInterX1.y()));}
                if(highInterX1.y()>=y1&&highInterX1.y()<=y2&&lowInterX1.y()!=highInterX1.y())     //high intersection is on the line with x=x1
                {m_dividedRegions[i-1][j-1].append(QPointF(x1,highInterX1.y()));}
                if((lowInterX1.y()<y1&&highInterX1.y()>y1)&&(lowInterY1.x()<x1&&highInterY1.x()>x1))   //point x1,y1 is inside the figure
                {m_dividedRegions[i-1][j-1].append(QPointF(x1,y1));}
                if(lowInterY1.x()>x1&&lowInterY1.x()<x2)                                            //low intersection is on the line with y=y1
                {m_dividedRegions[i-1][j-1].append(QPointF(lowInterY1.x(),y1));}
                if(highInterY1.x()>x1&&highInterY1.x()<x2&&lowInterY1.x()!=highInterY1.x())         //high intersection is on the line with y=y1
                {m_dividedRegions[i-1][j-1].append(QPointF(highInterY1.x(),y1));}
                if((lowInterX2.y()<y1&&highInterX2.y()>y1)&&(lowInterY1.x()<x2&&highInterY1.x()>x2))   //point x2,y1 is inside the figure
                {m_dividedRegions[i-1][j-1].append(QPointF(x2,y1));}
                if(lowInterX2.y()>=y1&&lowInterX2.y()<=y2)                                          //low intersection is on the line with x=x2
                {m_dividedRegions[i-1][j-1].append(QPointF(x2,lowInterX2.y()));}
                if(highInterX2.y()>=y1&&highInterX2.y()<=y2&&lowInterX2.y()!=highInterX2.y())       //high intersection is on the line with x=x2
                {m_dividedRegions[i-1][j-1].append(QPointF(x2,highInterX2.y()));}
                if((lowInterX2.y()<y2&&highInterX2.y()>y2)&&(lowInterY2.x()<x2&&highInterY2.x()>x2))   //point x2,y2 is inside the figure
                {m_dividedRegions[i-1][j-1].append(QPointF(x2,y2));}
                if(lowInterY2.x()>x1&&lowInterY2.x()<x2)                                            //low intersection is on the line with y=y2
                {m_dividedRegions[i-1][j-1].append(QPointF(lowInterY2.x(),y2));}
                if(highInterY2.x()>x1&&highInterY2.x()<x2&&lowInterY2.x()!=highInterY2.x())         //high intersection is on the line with y=y2
                {m_dividedRegions[i-1][j-1].append(QPointF(highInterY2.x(),y2));}
                if((lowInterX1.y()<y2&&highInterX1.y()>y2)&&(lowInterY2.x()<x1&&highInterY2.x()>x1))   //point x1,y2 is inside the figure
                {m_dividedRegions[i-1][j-1].append(QPointF(x1,y2));}
                for (int k=0; k<m_concretePoints.size(); ++k)
                {
                    if ((m_concretePoints[k].x()>x1&&m_concretePoints[k].x()<x2)&&(m_concretePoints[k].y()>y1&&m_concretePoints[k].y()<y2))
                    {
                        m_dividedRegions[i-1][j-1].append(QPointF(m_concretePoints[k].x(),m_concretePoints[k].y()));
                        int num=m_dividedRegions[i-1][j-1].size()-1;
                        if(!isBetween(m_dividedRegions[i-1][j-1][num],m_dividedRegions[i-1][j-1][0],m_dividedRegions[i-1][j-1][num-1]))
                        {
                            QPointF curPoint=m_dividedRegions[i-1][j-1][num];
                            int l=0;
                            do
                            {
                                m_dividedRegions[i-1][j-1][num-l]=m_dividedRegions[i-1][j-1][num-l-1];
                                m_dividedRegions[i-1][j-1][num-l-1]=curPoint;
                                ++l;
                            }
                            while((!isBetween(curPoint,m_dividedRegions[i-1][j-1][num-l+1],m_dividedRegions[i-1][j-1][num-l-1]))&&l>num);
                        }
                    }
                }
                qDebug()<<"Points of figure in column:"+QString::number(i)+" row:"+QString::number(j);
                for(int n=0; n<m_dividedRegions[i-1][j-1].size(); ++n)
                {
                    qDebug()<<"x:"+QString::number(m_dividedRegions[i-1][j-1][n].x())+" y:"+QString::number(m_dividedRegions[i-1][j-1][n].y());
                }
            }
        }
    }
    drawDivisions();
}

void Scene::slotNewSection()
{
    qDebug()<<"in new section method";
    int i=0;
    for (QGraphicsItem* item: this->items())
    {this->removeItem(item);
    qDebug()<<"item " + QString::number(i) + " removed";
    ++i;
    }
    m_concretePoints.erase(m_concretePoints.begin(),m_concretePoints.end());
    m_doneConcretePath=false;
    delete m_concretePath;
    m_concretePath=new QPainterPath();
    emit signalSceneCleared(true);
}

void Scene::slotDivideX(uint num)
{
    nXdivisions=num;
    qDebug()<<"number fo X axis divisions "+QString::number(num)+" received";
}

void Scene::slotDivideY(uint num)
{
    nYdivisions=num;
    qDebug()<<"number fo Y axis divisions "+QString::number(num)+" received";
    Divide();
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
    if (m_drawMode==LINE||m_drawMode==RECT)  //we are drawing a concrete section
    {m_concretePoints.append(point);}

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
        else if(m_drawMode==RECT)
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
            m_isRect=true;
//            if(m_concretePoints[0].x()<m_concretePoints[1].x()&&m_concretePoints[1].y()<m_concretePoints[2].y())
//            {m_leftToRight=true;}
//            else {m_leftToRight=false;}
//            qDebug()<<(m_leftToRight?"from left to right":"from right to left");
            qDebug()<<"draw mode is off";
            //remove points from scene because the rectangle is finished
            for (QGraphicsItem* item: m_pointsItems)
            {
                this->removeItem(item);
            }
            m_pointsItems.clear();
            emit signalDrawMode(false);
            emit signalSectDone(true);
            getSectSizes();
        }
        pen.setBrush(Qt::black);
        // pen.setWidth(1);
        brush.setColor(Qt::yellow); //не работает
        //this->addPolygon(m_concretePath->toFillPolygon(), pen, brush);
        this->addPath(*m_concretePath, pen, brush);
    }
    if (m_drawMode!=NONE)
    {
        if (m_drawMode==POINT)
        {   //we are drawing a point as a reinforcement bar
            pen.setBrush(QBrush(Qt::green));
            m_currentItem=this->addEllipse(point.x()-m_currDiam/2, point.y()-m_currDiam/2,m_currDiam,m_currDiam,pen);
            m_reinfItems.append(m_currentItem);
            m_reinfCircles.append(QPair<uint, QPointF>(m_currDiam, point));
        }
        else
        {   //we are drawing a point within concrete section
            pen.setBrush(QBrush(Qt::red));
            m_currentItem=this->addEllipse(point.x()-m_pointSize/2, point.y()-m_pointSize/2,m_pointSize,m_pointSize,pen);
            m_pointsItems.append(m_currentItem);
            //m_pointsGroup->addToGroup(m_currentItem);
        }
    }
}

void Scene::getSectSizes()
{
    lowX=m_concretePoints[0].x();
    highX=m_concretePoints[0].x();
    lowY=m_concretePoints[0].y();
    highY=m_concretePoints[0].y();
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
    //this->setSceneRect(lowX-m_viewMargin,lowY-m_viewMargin,m_recWidth+m_viewMargin, m_recHeight+m_viewMargin);

}

void Scene::drawDivisions()
{
    qDebug()<<"in Draw divisions method";
    pen.setBrush(Qt::blue);
    pen.setStyle(Qt::DotLine);
    // pen.setWidth(1);
    brush.setColor(Qt::yellow); //не работает
    if (m_isRect)       //simple rectangle
    {
        for (int i=1; i<=nXdivisions; ++i)
        {
            for (int j=1; j<=nYdivisions; ++j)
            {
                QRectF r=QRectF(m_dividedPoints[i-1][j-1].x(),m_dividedPoints[i-1][j-1].y(),(m_dividedPoints[i][j].x()-m_dividedPoints[i-1][j].x()),(m_dividedPoints[i][j].y()-m_dividedPoints[i][j-1].y()));
                m_divisionItems.append(this->addRect(r,pen,brush));
                qDebug()<<"x:"+QString::number(m_dividedPoints[i][j].x())+" y:"+ QString::number(m_dividedPoints[i][j].y());
            }
        }
    }
    else                //draw custom polygon
    {
        for (int i=0; i<nXdivisions; ++i)
        {
            for (int j=0; j<nYdivisions; ++j)
            {
                if (m_dividedRegions[i][j].size()!=0)
                {
                    qDebug()<<"drawing path at row:"+QString::number(i+1)<<"column:"<<QString::number(j);
                    QPainterPath p= QPainterPath(m_dividedRegions[i][j][0]);
                    qDebug()<<"Point added at x:"<<QString::number(m_dividedRegions[i][j][0].x())<<" y:"+QString::number(m_dividedRegions[i][j][0].y());
                    for(int n=1; n<m_dividedRegions[i][j].size(); ++n)
                    {
                        p.lineTo(m_dividedRegions[i][j][n]);
                        qDebug()<<"Point added at x:"<<QString::number(m_dividedRegions[i][j][n].x())<<" y:"+QString::number(m_dividedRegions[i][j][n].y());
                    }
                    p.lineTo(m_dividedRegions[i][j][0]);
                    m_divisionPaths.append(this->addPath(p,pen,brush));
                }
            }
        }
    }
    qDebug()<<"Draw divisions is finished";
    emit signalSceneDivided(true);
}

bool Scene::isBetween(QPointF p1, QPointF p2, QPointF p3)
{
    return (((p1.x()>p2.x()&&p1.x()<p3.x())||(p1.x()>p3.x()&&p1.x()<p2.x()))&&((p1.y()>p2.y()&&p1.y()<p3.y())||(p1.y()>p3.y()&&p1.y()<p2.y())));
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
    {
        if (m_drawMode!=NONE&&(m_doneConcretePath==false||m_doneReinforcement==false))
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
    emit signalCoordChanged(toSceneCoord(event->scenePos()));
    emit QGraphicsScene::mouseMoveEvent(event);
}

//void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
//{
//}


void Scene::timerEvent(QTimerEvent *event)
{
    emit signalSceneInit();
    killTimer(idTimer);
}
