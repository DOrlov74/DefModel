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
#include <QFileDialog>
#include <QMessageBox>
#include <QFont>

Scene::Scene(QWidget *parent) : QGraphicsScene(parent)
{
    m_concretePath=new QPainterPath();
    m_pathItem=nullptr;
    m_concretePath->setFillRule(Qt::WindingFill);
    //QObject::connect(this, SIGNAL(signalSceneInit()), this, SLOT(slotSceneInit()), Qt::QueuedConnection);
    //idTimer=startTimer(50);
    //emit signalGetRDiameter(m_currDiam);
    //qDebug()<<"Current diameter:"+QString::number(m_currDiam);
    myCalc=new Calculation(this);
    QObject::connect(this, SIGNAL(signalSetEb(double)), myCalc, SLOT(slotSetEb(double)));
    QObject::connect(this, SIGNAL(signalSetEs(double)), myCalc, SLOT(slotSetEs(double)));
    QObject::connect(this, SIGNAL(signalSetRb(double)), myCalc, SLOT(slotSetRb(double)));
    QObject::connect(this, SIGNAL(signalSetRbt(double)), myCalc, SLOT(slotSetRbt(double)));
    QObject::connect(this, SIGNAL(signalSetRs(double)), myCalc, SLOT(slotSetRs(double)));
    QObject::connect(myCalc, SIGNAL(signalCalcStart()), this, SLOT(slotCalcStart()));
    QObject::connect(myCalc, SIGNAL(signalPercentChanged(int)), this, SLOT(slotPercentChanged(int)));
    QObject::connect(myCalc, SIGNAL(signalCalcEnd(bool)), this, SLOT(slotCalcEnd(bool)));
    QObject::connect(myCalc, SIGNAL(signalDrawStress()), this, SLOT(slotDrawStress()));
    QObject::connect(myCalc, SIGNAL(signalExportStart()), this, SLOT(slotExportStart()));
    QObject::connect(myCalc, SIGNAL(signalExportPercentChanged(int)), this, SLOT(slotExportPercentChanged(int)));
    QObject::connect(myCalc, SIGNAL(signalExportEnd()), this, SLOT(slotExportEnd()));
}

Scene::~Scene()
{
    delete m_concretePath;
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
    if ((!m_doneReinforcement)||(!m_doneConcretePath)||(!m_sectDivided))
    {
        QString message;
        if (!m_doneReinforcement)
        {
            message="Draw Reinforcement first";
        }
        else if (!m_doneConcretePath)
        {
            message="Draw Concrete section first";
        }
        else
        {
            message="Divide Concrete section first";
        }
        QMessageBox::warning(nullptr,"Warning",message);
    }
    else
    {
        qDebug()<<"in Calculate Slot";
        myCalc->setXdivision(nXdivisions);
        myCalc->setYdivision(nYdivisions);
        m_concreteArea.fill(QVector<double>(),nXdivisions);
        m_concreteJx.fill(QVector<double>(),nXdivisions);
        m_concreteJy.fill(QVector<double>(),nXdivisions);
        m_concreteCenter.fill(QVector<QPointF>(),nXdivisions);
        double dWidth;
        double dHeight;
        if (m_isRect)
        {
            for (int i=1; i<m_dividedPoints.size(); ++i)
            {
                m_concreteArea[i-1].fill(0,nYdivisions);
                m_concreteJx[i-1].fill(0,nYdivisions);
                m_concreteJy[i-1].fill(0,nYdivisions);
                m_concreteCenter[i-1].fill(QPointF(0,0),nYdivisions);
                for (int j=1; j<m_dividedPoints[i].size();++j)
                {
                    dWidth=qSqrt(qPow((m_dividedPoints[i][j-1].x()-m_dividedPoints[i-1][j-1].x()),2)+qPow((m_dividedPoints[i][j-1].y()-m_dividedPoints[i-1][j-1].y()),2));
                    dHeight=qSqrt(qPow((m_dividedPoints[i-1][j].x()-m_dividedPoints[i-1][j-1].x()),2)+qPow((m_dividedPoints[i-1][j].y()-m_dividedPoints[i-1][j-1].y()),2));
                    m_concreteArea[i-1][j-1]=dWidth*dHeight;
                    m_concreteJx[i-1][j-1]=dWidth*qPow(dHeight,3)/12;
                    m_concreteJy[i-1][j-1]=qPow(dWidth,3)*dHeight/12;
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
            double dXmin=0;
            double dXmax=0;
            double dYmin=0;
            double dYmax=0;
            int iSize;
            bool JxIsSet;
            for (int i=0; i<m_dividedRegions.size(); ++i)
            {
                m_concreteArea[i].fill(0,nYdivisions);
                m_concreteJx[i].fill(0,nYdivisions);
                m_concreteJy[i].fill(0,nYdivisions);
                m_concreteCenter[i].fill(QPointF(0,0),nYdivisions);
                for (int j=0; j<m_dividedRegions[i].size();++j)
                {
                    JxIsSet=false;
                    iSize=m_dividedRegions[i][j].size();
                    for (int k=0; k<iSize;++k)
                    {
                        if (k!=iSize-1)
                        {
                            dSum1+=m_dividedRegions[i][j][k].x()*m_dividedRegions[i][j][k+1].y();
                            dSum2+=m_dividedRegions[i][j][k+1].x()*m_dividedRegions[i][j][k].y();
                        }
                        dSumX+=m_dividedRegions[i][j][k].x();
                        dSumY+=m_dividedRegions[i][j][k].y();
                        if (m_dividedRegions[i][j][k].x()<dXmin)
                        {dXmin=m_dividedRegions[i][j][k].x();}
                        if (m_dividedRegions[i][j][k].x()>dXmax)
                        {dXmax=m_dividedRegions[i][j][k].x();}
                        if (m_dividedRegions[i][j][k].y()<dYmin)
                        {dYmin=m_dividedRegions[i][j][k].y();}
                        if (m_dividedRegions[i][j][k].y()>dYmax)
                        {dYmax=m_dividedRegions[i][j][k].y();}
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
                        dWidth=dXmax-dXmin;
                        dHeight=dYmax-dYmin;
                        bool freeFaces[4];
                        for (int f=0; f<4;++f)
                        {
                            if (m_dividedFaces[i][j][f].empty())        //check if there is no points on the face
                            {freeFaces[f]=true;}
                        }
                        switch (iSize)
                        {
                        case 3:                 //it's a triangle
                            if (!freeFaces[3]&&!freeFaces[0])           //triangle lies in the corner with xmin and ymax
                            {
                                m_concreteJx[i][j]=dWidth*qPow(dHeight,3)/36;
                                m_concreteJy[i][j]=qPow(dWidth,3)*dHeight/36;
                                JxIsSet=true;
                            }
                            else if (JxIsSet==false)
                            {
                                for (int f=1; f<4;++f)
                                {
                                    if (!freeFaces[f-1]&&!freeFaces[f])       //triangle lies in the corner
                                    {
                                        m_concreteJx[i][j]=dWidth*qPow(dHeight,3)/36;
                                        m_concreteJy[i][j]=qPow(dWidth,3)*dHeight/36;
                                        JxIsSet=true;
                                        break;
                                    }
                                }
                                //triangle lies only on one side
                                if (JxIsSet==false)
                                {
                                    QPointF p1;                     //point of triangle which is not lying on any face
                                    for (int k=0; k<iSize; ++k)
                                    {
                                        for (int f=0; f<4;++f)
                                        {
                                            if (!freeFaces[f]&&m_dividedFaces[i][j][k].size()==2)       //two points of three are on this face
                                            {
                                                if((k+1)!=m_dividedFaces[i][j][k][0]&&(k+1)!=m_dividedFaces[i][j][k][1])        //we need a point which is not on the base face
                                                {
                                                    p1=m_dividedRegions[i][j][k];
                                                    qDebug()<<"Point which is not on the base face is found";
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    if (!freeFaces[1]||!freeFaces[3])     //triangle lies on X axis
                                    {
                                        double b1=p1.x()-dXmin;                     //we are going to divide triangle by p1
                                        double b2=dXmax-p1.x();
                                        double xc1=m_concreteCenter[i][j].x()-dXmin-2*b1/3;     //distance from first triagle center to figure center
                                        double xc2=dXmax-m_concreteCenter[i][j].x()-2*b2/3;     //distance from second triagle center to figure center
                                        m_concreteJx[i][j]=dWidth*qPow(dHeight,3)/36;
                                        m_concreteJy[i][j]=qPow(b1,3)*dHeight/36+b1*dHeight/2*qPow(xc1,2)+qPow(b2,3)*dHeight/36+b2*dHeight/2*qPow(xc2,2);
                                        JxIsSet=true;
                                    }
                                    else     //triangle lies on Y axis
                                    {
                                        double h1=p1.y()-dYmin;                     //we are going to divide triangle by p1
                                        double h2=dYmax-p1.y();
                                        double yc1=m_concreteCenter[i][j].y()-dYmin-2*h1/3;     //distance from first triagle center to figure center
                                        double yc2=dYmax-m_concreteCenter[i][j].y()-2*h2/3;     //distance from second triagle center to figure center
                                        m_concreteJx[i][j]=dWidth*qPow(h1,3)/36+h1*dWidth/2*qPow(yc1,2)+dWidth*qPow(h2,3)/36+h2*dWidth/2*qPow(yc2,2);
                                        m_concreteJy[i][j]=qPow(dWidth,3)*dHeight/36;
                                        JxIsSet=true;
                                    }
                                }
                            }
                            break;
                        case 4:                     //it's a quadrangle
                            int nFreeFaces=0;
                            for (int f=0; f<4;++f)
                            {
                                if (freeFaces[f])
                                {++nFreeFaces;}
                            }
                            if (nFreeFaces==0)         //rectange fill all figure
                            {
                                m_concreteJx[i][j]=dWidth*qPow(dHeight,3)/12;
                                m_concreteJy[i][j]=qPow(dWidth,3)*dHeight/12;
                                JxIsSet=true;
                            }
                            else if (nFreeFaces==1)     //only one face is free
                            {
                                QPointF p1;                             //point of quadrangle for divide it
                                if (freeFaces[0]||freeFaces[2])         //free face is on Xmin or on Xmax
                                {
                                    for (int k=0; k<iSize; ++k)
                                    {
                                        if (m_dividedRegions[i][j][k].x()!=dXmax&&m_dividedRegions[i][j][k].x()!=dXmin)
                                        {
                                            p1=m_dividedRegions[i][j][k];
                                            qDebug()<<"Point for divide quadrangle is found";
                                            break;
                                        }
                                    }
                                    double w1=p1.x()-dXmin;                                 //we are going to divide quadrangle by p1
                                    double w2=dXmax-p1.x();
                                    if (freeFaces[0])       //free face is on Xmin
                                    {
                                        double xc1=m_concreteCenter[i][j].x()-dXmin-2*w1/3;     //distance along X axis from first triangle center to figure center
                                        double xc2=dXmax-m_concreteCenter[i][j].x()-w2/2;       //distance along X axis from second rectangle center to figure center
                                        double yc1=m_concreteCenter[i][j].y()-dYmin-dHeight/3;        //distance along Y axis from first triangle center to figure center
                                        double yc2=dYmax-m_concreteCenter[i][j].y()-dHeight/2;        //distance along Y axis from second rectangle center to figure center
                                        m_concreteJx[i][j]=w1*qPow(dHeight,3)/36+w1*dHeight/2*qPow(yc1,2)+w2*qPow(dHeight,3)/12+w2*dHeight*qPow(yc2,2);
                                        m_concreteJy[i][j]=qPow(w1,3)*dHeight/36+w1*dHeight/2*qPow(xc1,2)+dHeight*qPow(w2,3)/12+w2*dHeight*qPow(xc2,2);
                                        JxIsSet=true;
                                    }
                                    else if (freeFaces[2])      //free face is on Xmax
                                    {
                                        double xc1=m_concreteCenter[i][j].x()-dXmin-w1/2;       //distance along X axis from first rectangle center to figure center
                                        double xc2=dXmax-m_concreteCenter[i][j].x()-2*w2/3;     //distance along X axis from second triangle center to figure center
                                        double yc1=m_concreteCenter[i][j].y()-dYmin-dHeight/2;        //distance along Y axis from first rectangle center to figure center
                                        double yc2=dYmax-m_concreteCenter[i][j].y()-2*dHeight/3;        //distance along Y axis from second triangle center to figure center
                                        m_concreteJx[i][j]=w1*qPow(dHeight,3)/12+w1*dHeight/2*qPow(yc1,2)+w2*qPow(dHeight,3)/36+w2*dHeight*qPow(yc2,2);
                                        m_concreteJy[i][j]=qPow(w1,3)*dHeight/12+w1*dHeight/2*qPow(xc1,2)+dHeight*qPow(w2,3)/36+w2*dHeight*qPow(xc2,2);
                                        JxIsSet=true;
                                    }
                                }
                                else if (freeFaces[1]||freeFaces[3])         //free face is on Ymin or on Ymax
                                {
                                    for (int k=0; k<iSize; ++k)
                                    {
                                        if (m_dividedRegions[i][j][k].y()!=dYmax&&m_dividedRegions[i][j][k].y()!=dYmin)
                                        {
                                            p1=m_dividedRegions[i][j][k];
                                            qDebug()<<"Point for divide quadrangle is found";
                                            break;
                                        }
                                    }
                                    double h1=p1.y()-dYmin;                                 //we are going to divide quadrangle by p1
                                    double h2=dYmax-p1.y();
                                    if (freeFaces[1])       //free face is on Ymin
                                    {
                                        double xc1=m_concreteCenter[i][j].x()-dXmin-2*dWidth/3;        //distance along X axis from first triangle center to figure center
                                        double xc2=dXmax-m_concreteCenter[i][j].x()-dWidth/2;        //distance along X axis from second rectangle center to figure center
                                        double yc1=m_concreteCenter[i][j].y()-dYmin-2*h1/3;         //distance along Y axis from first triangle center to figure center
                                        double yc2=dYmax-m_concreteCenter[i][j].y()-h2/2;           //distance along Y axis from second rectangle center to figure center
                                        m_concreteJx[i][j]=dWidth*qPow(h1,3)/36+dWidth*h1/2*qPow(yc1,2)+dWidth*qPow(h2,3)/12+dWidth*h2*qPow(yc2,2);
                                        m_concreteJy[i][j]=qPow(dWidth,3)*h1/36+dWidth*h1/2*qPow(xc1,2)+qPow(dWidth,3)*h2/12+dWidth*h2*qPow(xc2,2);
                                        JxIsSet=true;
                                    }
                                    else if (freeFaces[2])      //free face is on Ymax
                                    {
                                        double xc1=m_concreteCenter[i][j].x()-dXmin-dWidth/2;           //distance along X axis from first rectangle center to figure center
                                        double xc2=dXmax-m_concreteCenter[i][j].x()-2*dWidth/3;        //distance along X axis from second triangle center to figure center
                                        double yc1=m_concreteCenter[i][j].y()-dYmin-h1/2;           //distance along Y axis from first rectangle center to figure center
                                        double yc2=dYmax-m_concreteCenter[i][j].y()-2*h2/3;         //distance along Y axis from second triangle center to figure center
                                        m_concreteJx[i][j]=dWidth*qPow(h1,3)/12+dWidth*h1/2*qPow(yc1,2)+dWidth*qPow(h2,3)/36+dWidth*h2*qPow(yc2,2);
                                        m_concreteJy[i][j]=qPow(dWidth,3)*h1/12+dWidth*h1/2*qPow(xc1,2)+qPow(dWidth,3)*h2/36+dWidth*h2*qPow(xc2,2);
                                        JxIsSet=true;
                                    }
                                }
                            }
                            else if (nFreeFaces==2)     //two faces are free
                            {
                                QPointF p1;                             //point of quadrangle for divide it
                                for (int k=0; k<iSize; ++k)             //iterate through polygon points
                                {
                                    for (int f=0; f<4; ++f)             //iterate through divided rectangle faces
                                    {
                                        if (!freeFaces[f])
                                        {
                                            for (int p=0; p<m_dividedFaces[i][j][f].size(); ++p)
                                            {
                                                if (m_dividedFaces[i][j][f][p]!=(k+1))
                                                {
                                                    p1=m_dividedRegions[i][j][k];
                                                    qDebug()<<"Point for divide quadrangle is found";
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                                double h1, w1, h2, w2;                  //height and width of triangles
                                double yc1, yc2, yc3, xc1, xc2, xc3;    //distances from figure center to divided elements
                                double Jx1, Jx2, Jx3, Jy1, Jy2, Jy3;    //moments of inertia of divided elements
                                if (freeFaces[0]&&freeFaces[1])         //free faces are with Xmin and Ymin
                                {
                                    h1=dYmax-p1.y();             //height of triange connected to face with Ymax
                                    w2=dXmax-p1.x();             //width of triangle connected to face with Xmax
                                    yc1=dYmax-m_concreteCenter[i][j].y()-h1/3;       //distance along Y axis from the first triangle center to the section center
                                    xc2=dXmax-m_concreteCenter[i][j].x()-w2/3;       //distance along X axis from the second triangle center to the section center
                                    yc3=dYmax-m_concreteCenter[i][j].y()-h1/2;       //distance along Y axis from the rectangle center to the section center
                                    xc3=dXmax-m_concreteCenter[i][j].x()-w2/2;       //distance along X axis from the rectangle center to the section center
                                    Jx3=w2*qPow(h1,3)/12+w2*h1*qPow(yc3,2);          //moment of inertia about X axis of the rectangle
                                    Jy3=qPow(w2,3)*h1/12+w2*h1*qPow(xc3,2);          //moment of inertia about Y axis of the rectangle
                                    QPointF pXmin, pYmin;
                                    if (p1.x()>dXmin)                   //point is not on Xmin
                                    {
                                        w1=p1.x()-dXmin;                //width of triangle connected to face with Ymax
                                        xc1=m_concreteCenter[i][j].x()-p1.x()+w1/3;         //distance along X axis from the first triangle center to the section center
                                        Jx1=w1*qPow(h1,3)/36+w1*h1/2*qPow(yc1,2);           //moment of inertia about X axis of the first triangle
                                        Jy1=qPow(w1,3)*h1/36+w1*h1/2*qPow(xc1,2);           //moment of inertia about Y axis of the first triangle
                                    }
                                    else                                //point is on Xmin
                                    {
                                        pXmin=m_dividedRegions[i][j][m_dividedFaces[i][j][3][0]-1];
                                        for (int p=1; p<m_dividedFaces[i][j][3].size(); ++p)
                                        {
                                            if(m_dividedRegions[i][j][m_dividedFaces[i][j][3][p]-1].x()<m_dividedRegions[i][j][m_dividedFaces[i][j][3][p-1]-1].x())
                                            {
                                                pXmin=m_dividedRegions[i][j][m_dividedFaces[i][j][3][p]-1];
                                            }
                                        }
                                        w1=pXmin.x()-p1.x();            //width of triangle connected to face with Ymax
                                        xc1=m_concreteCenter[i][j].x()-p1.x()-w1/3;         //distance along X axis from the first triangle center to the section center
                                        Jx1=-w1*qPow(h1,3)/36-w1*h1/2*qPow(yc1,2);           //moment of inertia about X axis of the first triangle
                                        Jy1=-qPow(w1,3)*h1/36-w1*h1/2*qPow(xc1,2);           //moment of inertia about Y axis of the first triangle
                                    }
                                    if (p1.y()>dYmin)                   //point is not on Ymin
                                    {
                                        h2=p1.y()-dYmin;                //height of triangle connected to face with Xmax
                                        yc2=m_concreteCenter[i][j].y()-p1.y()+h2/3;         //distance along Y axis from the second triangle center to the section center
                                        Jx2=w2*qPow(h2,3)/36+w2*h2/2*qPow(yc2,2);           //moment of inertia about X axis of the second triangle
                                        Jy2=qPow(w2,3)*h2/36+w2*h2/2*qPow(xc2,2);           //moment of inertia about Y axis of the second triangle
                                    }
                                    else                                //point is on Ymin
                                    {
                                        pYmin=m_dividedRegions[i][j][m_dividedFaces[i][j][2][0]-1];
                                        for (int p=1; p<m_dividedFaces[i][j][2].size(); ++p)
                                        {
                                            if(m_dividedRegions[i][j][m_dividedFaces[i][j][2][p]-1].y()<m_dividedRegions[i][j][m_dividedFaces[i][j][2][p-1]-1].y())
                                            {
                                                pYmin=m_dividedRegions[i][j][m_dividedFaces[i][j][2][p]-1];
                                            }
                                        }
                                        h2=pYmin.y()-p1.y();            //height of triangle connected to face with Xmax
                                        yc2=m_concreteCenter[i][j].y()-p1.y()-h2/3;         //distance along Y axis from the second triangle center to the section center
                                        Jx2=-w2*qPow(h2,3)/36-w2*h2/2*qPow(yc2,2);           //moment of inertia about X axis of the second triangle
                                        Jy2=-qPow(w2,3)*h2/36-w2*h2/2*qPow(xc2,2);           //moment of inertia about Y axis of the second triangle
                                    }
                                }
                                else if (freeFaces[1]&&freeFaces[2])         //free faces are with Xmax and Ymin
                                {
                                    h1=dYmax-p1.y();             //height of triange connected to face with Ymax
                                    w2=p1.x()-dXmin;             //width of triangle connected to face with Xmin
                                    yc1=dYmax-m_concreteCenter[i][j].y()-h1/3;       //distance along Y axis from the first triangle center to the section center
                                    xc2=m_concreteCenter[i][j].x()-dXmin-w2/3;       //distance along X axis from the second triangle center to the section center
                                    yc3=dYmax-m_concreteCenter[i][j].y()-h1/2;       //distance along Y axis from the rectangle center to the section center
                                    xc3=m_concreteCenter[i][j].x()-dXmin-w2/2;       //distance along X axis from the rectangle center to the section center
                                    Jx3=w2*qPow(h1,3)/12+w2*h1*qPow(yc3,2);          //moment of inertia about X axis of the rectangle
                                    Jy3=qPow(w2,3)*h1/12+w2*h1*qPow(xc3,2);          //moment of inertia about Y axis of the rectangle
                                    QPointF pXmax, pYmin;
                                    if (p1.x()<dXmax)                   //point is not on Xmax
                                    {
                                        w1=dXmax-p1.x();                //width of triangle connected to face with Ymax
                                        xc1=p1.x()-m_concreteCenter[i][j].x()+w1/3;         //distance along X axis from the first triangle center to the section center
                                        Jx1=w1*qPow(h1,3)/36+w1*h1/2*qPow(yc1,2);           //moment of inertia about X axis of the first triangle
                                        Jy1=qPow(w1,3)*h1/36+w1*h1/2*qPow(xc1,2);           //moment of inertia about Y axis of the first triangle
                                    }
                                    else                                //point is on Xmax
                                    {
                                        pXmax=m_dividedRegions[i][j][m_dividedFaces[i][j][3][0]-1];
                                        for (int p=1; p<m_dividedFaces[i][j][3].size(); ++p)
                                        {
                                            if(m_dividedRegions[i][j][m_dividedFaces[i][j][3][p]-1].x()>m_dividedRegions[i][j][m_dividedFaces[i][j][3][p-1]-1].x())
                                            {
                                                pXmax=m_dividedRegions[i][j][m_dividedFaces[i][j][3][p]-1];
                                            }
                                        }
                                        w1=p1.x()-pXmax.x();            //width of triangle connected to face with Ymax
                                        xc1=dXmax-m_concreteCenter[i][j].x()-w1/3;         //distance along X axis from the first triangle center to the section center
                                        Jx1=-w1*qPow(h1,3)/36-w1*h1/2*qPow(yc1,2);           //moment of inertia about X axis of the first triangle
                                        Jy1=-qPow(w1,3)*h1/36-w1*h1/2*qPow(xc1,2);           //moment of inertia about Y axis of the first triangle
                                    }
                                    if (p1.y()>dYmin)                   //point is not on Ymin
                                    {
                                        h2=p1.y()-dYmin;                //height of triangle connected to face with Xmax
                                        yc2=m_concreteCenter[i][j].y()-p1.y()+h2/3;         //distance along Y axis from the second triangle center to the section center
                                        Jx2=w2*qPow(h2,3)/36+w2*h2/2*qPow(yc2,2);           //moment of inertia about X axis of the second triangle
                                        Jy2=qPow(w2,3)*h2/36+w2*h2/2*qPow(xc2,2);           //moment of inertia about Y axis of the second triangle
                                    }
                                    else                                //point is on Ymin
                                    {
                                        pYmin=m_dividedRegions[i][j][m_dividedFaces[i][j][0][0]-1];
                                        for (int p=1; p<m_dividedFaces[i][j][0].size(); ++p)
                                        {
                                            if(m_dividedRegions[i][j][m_dividedFaces[i][j][0][p]-1].y()<m_dividedRegions[i][j][m_dividedFaces[i][j][0][p-1]-1].y())
                                            {
                                                pYmin=m_dividedRegions[i][j][m_dividedFaces[i][j][0][p]-1];
                                            }
                                        }
                                        h2=pYmin.y()-p1.y();            //height of triangle connected to face with Xmin
                                        yc2=m_concreteCenter[i][j].y()-p1.y()-h2/3;         //distance along Y axis from the second triangle center to the section center
                                        Jx2=-w2*qPow(h2,3)/36-w2*h2/2*qPow(yc2,2);           //moment of inertia about X axis of the second triangle
                                        Jy2=-qPow(w2,3)*h2/36-w2*h2/2*qPow(xc2,2);           //moment of inertia about Y axis of the second triangle
                                    }
                                }
                                else if (freeFaces[2]&&freeFaces[3])         //free faces are with Xmax and Ymax
                                {
                                    h1=p1.y()-dYmin;             //height of triange connected to face with Ymin
                                    w2=p1.x()-dXmin;             //width of triangle connected to face with Xmin
                                    yc1=m_concreteCenter[i][j].y()-dYmin-h1/3;       //distance along Y axis from the first triangle center to the section center
                                    xc2=m_concreteCenter[i][j].x()-dXmin-w2/3;       //distance along X axis from the second triangle center to the section center
                                    yc3=m_concreteCenter[i][j].y()-dYmin-h1/2;       //distance along Y axis from the rectangle center to the section center
                                    xc3=m_concreteCenter[i][j].x()-dXmin-w2/2;       //distance along X axis from the rectangle center to the section center
                                    Jx3=w2*qPow(h1,3)/12+w2*h1*qPow(yc3,2);          //moment of inertia about X axis of the rectangle
                                    Jy3=qPow(w2,3)*h1/12+w2*h1*qPow(xc3,2);          //moment of inertia about Y axis of the rectangle
                                    QPointF pXmax, pYmax;
                                    if (p1.x()<dXmax)                   //point is not on Xmax
                                    {
                                        w1=dXmax-p1.x();                //width of triangle connected to face with Ymin
                                        xc1=p1.x()-m_concreteCenter[i][j].x()+w1/3;         //distance along X axis from the first triangle center to the section center
                                        Jx1=w1*qPow(h1,3)/36+w1*h1/2*qPow(yc1,2);           //moment of inertia about X axis of the first triangle
                                        Jy1=qPow(w1,3)*h1/36+w1*h1/2*qPow(xc1,2);           //moment of inertia about Y axis of the first triangle
                                    }
                                    else                                //point is on Xmax
                                    {
                                        pXmax=m_dividedRegions[i][j][m_dividedFaces[i][j][1][0]-1];
                                        for (int p=1; p<m_dividedFaces[i][j][1].size(); ++p)
                                        {
                                            if(m_dividedRegions[i][j][m_dividedFaces[i][j][1][p]-1].x()>m_dividedRegions[i][j][m_dividedFaces[i][j][1][p-1]-1].x())
                                            {
                                                pXmax=m_dividedRegions[i][j][m_dividedFaces[i][j][1][p]-1];
                                            }
                                        }
                                        w1=p1.x()-pXmax.x();            //width of triangle connected to face with Ymin
                                        xc1=dXmax-m_concreteCenter[i][j].x()-w1/3;         //distance along X axis from the first triangle center to the section center
                                        Jx1=-w1*qPow(h1,3)/36-w1*h1/2*qPow(yc1,2);           //moment of inertia about X axis of the first triangle
                                        Jy1=-qPow(w1,3)*h1/36-w1*h1/2*qPow(xc1,2);           //moment of inertia about Y axis of the first triangle
                                    }
                                    if (p1.y()<dYmax)                   //point is not on Ymax
                                    {
                                        h2=dYmax-p1.y();                //height of triangle connected to face with Xmin
                                        yc2=p1.y()-m_concreteCenter[i][j].y()+h2/3;         //distance along Y axis from the second triangle center to the section center
                                        Jx2=w2*qPow(h2,3)/36+w2*h2/2*qPow(yc2,2);           //moment of inertia about X axis of the second triangle
                                        Jy2=qPow(w2,3)*h2/36+w2*h2/2*qPow(xc2,2);           //moment of inertia about Y axis of the second triangle
                                    }
                                    else                                //point is on Ymax
                                    {
                                        pYmax=m_dividedRegions[i][j][m_dividedFaces[i][j][0][0]-1];
                                        for (int p=1; p<m_dividedFaces[i][j][0].size(); ++p)
                                        {
                                            if(m_dividedRegions[i][j][m_dividedFaces[i][j][0][p]-1].y()>m_dividedRegions[i][j][m_dividedFaces[i][j][0][p-1]-1].y())
                                            {
                                                pYmax=m_dividedRegions[i][j][m_dividedFaces[i][j][0][p]-1];
                                            }
                                        }
                                        h2=p1.y()-pYmax.y();            //height of triangle connected to face with Xmin
                                        yc2=p1.y()-m_concreteCenter[i][j].y()-h2/3;         //distance along Y axis from the second triangle center to the section center
                                        Jx2=-w2*qPow(h2,3)/36-w2*h2/2*qPow(yc2,2);           //moment of inertia about X axis of the second triangle
                                        Jy2=-qPow(w2,3)*h2/36-w2*h2/2*qPow(xc2,2);           //moment of inertia about Y axis of the second triangle
                                    }
                                }
                                else //if (freeFaces[0]&&freeFaces[3])         //free faces are with Xmin and Ymax
                                {
                                    h1=p1.y()-dYmin;             //height of triange connected to face with Ymin
                                    w2=dXmax-p1.x();             //width of triangle connected to face with Xmax
                                    yc1=m_concreteCenter[i][j].y()-dYmin-h1/3;       //distance along Y axis from the first triangle center to the section center
                                    xc2=dXmax-m_concreteCenter[i][j].x()-w2/3;       //distance along X axis from the second triangle center to the section center
                                    yc3=m_concreteCenter[i][j].y()-dYmin-h1/2;       //distance along Y axis from the rectangle center to the section center
                                    xc3=dXmax-m_concreteCenter[i][j].x()-w2/2;       //distance along X axis from the rectangle center to the section center
                                    Jx3=w2*qPow(h1,3)/12+w2*h1*qPow(yc3,2);          //moment of inertia about X axis of the rectangle
                                    Jy3=qPow(w2,3)*h1/12+w2*h1*qPow(xc3,2);          //moment of inertia about Y axis of the rectangle
                                    QPointF pXmin, pYmax;
                                    if (p1.x()>dXmin)                   //point is not on Xmin
                                    {
                                        w1=p1.x()-dXmin;                //width of triangle connected to face with Ymin
                                        xc1=m_concreteCenter[i][j].x()-p1.x()+w1/3;         //distance along X axis from the first triangle center to the section center
                                        Jx1=w1*qPow(h1,3)/36+w1*h1/2*qPow(yc1,2);           //moment of inertia about X axis of the first triangle
                                        Jy1=qPow(w1,3)*h1/36+w1*h1/2*qPow(xc1,2);           //moment of inertia about Y axis of the first triangle
                                    }
                                    else                                //point is on Xmin
                                    {
                                        pXmin=m_dividedRegions[i][j][m_dividedFaces[i][j][1][0]-1];
                                        for (int p=1; p<m_dividedFaces[i][j][1].size(); ++p)
                                        {
                                            if(m_dividedRegions[i][j][m_dividedFaces[i][j][1][p]-1].x()<m_dividedRegions[i][j][m_dividedFaces[i][j][1][p-1]-1].x())
                                            {
                                                pXmin=m_dividedRegions[i][j][m_dividedFaces[i][j][1][p]-1];
                                            }
                                        }
                                        w1=pXmin.x()-p1.x();            //width of triangle connected to face with Ymin
                                        xc1=m_concreteCenter[i][j].x()-p1.x()-w1/3;         //distance along X axis from the first triangle center to the section center
                                        Jx1=-w1*qPow(h1,3)/36-w1*h1/2*qPow(yc1,2);           //moment of inertia about X axis of the first triangle
                                        Jy1=-qPow(w1,3)*h1/36-w1*h1/2*qPow(xc1,2);           //moment of inertia about Y axis of the first triangle
                                    }
                                    if (p1.y()<dYmax)                   //point is not on Ymax
                                    {
                                        h2=dYmax-p1.y();                //height of triangle connected to face with Xmax
                                        yc2=p1.y()-m_concreteCenter[i][j].y()+h2/3;         //distance along Y axis from the second triangle center to the section center
                                        Jx2=w2*qPow(h2,3)/36+w2*h2/2*qPow(yc2,2);           //moment of inertia about X axis of the second triangle
                                        Jy2=qPow(w2,3)*h2/36+w2*h2/2*qPow(xc2,2);           //moment of inertia about Y axis of the second triangle
                                    }
                                    else                                //point is on Ymax
                                    {
                                        pYmax=m_dividedRegions[i][j][m_dividedFaces[i][j][2][0]-1];
                                        for (int p=1; p<m_dividedFaces[i][j][2].size(); ++p)
                                        {
                                            if(m_dividedRegions[i][j][m_dividedFaces[i][j][2][p]-1].y()>m_dividedRegions[i][j][m_dividedFaces[i][j][2][p-1]-1].y())
                                            {
                                                pYmax=m_dividedRegions[i][j][m_dividedFaces[i][j][2][p]-1];
                                            }
                                        }
                                        h2=p1.y()-pYmax.y();            //height of triangle connected to face with Xmax
                                        yc2=p1.y()-m_concreteCenter[i][j].y()-h2/3;         //distance along Y axis from the second triangle center to the section center
                                        Jx2=-w2*qPow(h2,3)/36-w2*h2/2*qPow(yc2,2);           //moment of inertia about X axis of the second triangle
                                        Jy2=-qPow(w2,3)*h2/36-w2*h2/2*qPow(xc2,2);           //moment of inertia about Y axis of the second triangle
                                    }
                                }
                                m_concreteJx[i][j]=Jx1+Jx2+Jx3;
                                m_concreteJy[i][j]=Jy1+Jy2+Jy3;
                                JxIsSet=true;
                            }
                            else //if (nFreeFaces==3)     //three faces are free
                            {
                                QPointF p1, p2;                             //points of quadrangle for divide it
                                bool p1IsFound=false;
                                for (int f=0; f<4; ++f)             //iterate through divided rectangle faces
                                {
                                    if (!freeFaces[f])
                                    {
                                        for (int k=0; k<iSize; ++k)             //iterate through polygon points
                                        {
                                            int p=0;
                                            while (p<m_dividedFaces[i][j][f].size()&&m_dividedFaces[i][j][f][p]==(k+1))
                                            {
                                                ++p;
                                            }
                                            if (p<m_dividedFaces[i][j][f].size())
                                            {
                                                if (!p1IsFound)
                                                {
                                                    p1=m_dividedRegions[i][j][k];
                                                    qDebug()<<"Point one for divide quadrangle is found";
                                                    p1IsFound=true;
                                                }
                                                else
                                                {
                                                    p2=m_dividedRegions[i][j][k];
                                                    qDebug()<<"Point two for divide quadrangle is found";
                                                }
                                            }
                                        }
                                    }
                                }
                                double h1, w1, h2, w2, h3, w3;                  //height and width of triangles
                                double yc1, yc2, yc3, yc4, xc1, xc2, xc3, xc4;    //distances from figure center to divided elements
                                double Jx1, Jx2, Jx3, Jx4, Jy1, Jy2, Jy3, Jy4;    //moments of inertia of divided elements
                                if (!freeFaces[0])         //face with connection to figure is with Xmin
                                {
                                    if (p1.y()>p2.y())
                                    {
                                        switchPoint(p1, p2);
                                    }
                                    h1=p2.y()-p1.y();             //height of the first triange between two points
                                    w2=p1.x()-dXmin;             //width of the second triangle connected to face with Xmin
                                    w3=p2.x()-dXmin;             //width of the third triangle connected to face with Xmin
                                    yc1=p2.y()-m_concreteCenter[i][j].y()-h1/3;       //distance along Y axis from the first triangle center to the section center
                                    xc2=m_concreteCenter[i][j].x()-dXmin-w2/3;       //distance along X axis from the second triangle center to the section center
                                    xc3=m_concreteCenter[i][j].x()-dXmin-w3/3;       //distance along X axis from the third triangle center to the section center
                                    yc4=p2.y()-m_concreteCenter[i][j].y()-h1/2;       //distance along Y axis from the rectangle center to the section center
                                    xc4=p1.x()-m_concreteCenter[i][j].x()-w2/2;       //distance along X axis from the rectangle center to the section center
                                    Jx4=w2*qPow(h1,3)/12+w2*h1*qPow(yc4,2);          //moment of inertia about X axis of the rectangle
                                    Jy4=qPow(w2,3)*h1/12+w2*h1*qPow(xc4,2);          //moment of inertia about Y axis of the rectangle
                                    QPointF pYmin, pYmax;
                                    if (p1.x()<dXmax)                   //point is not on Xmax
                                    {
                                        w1=dXmax-p1.x();                //width of triangle between two points
                                        xc1=p1.x()-m_concreteCenter[i][j].x()+w1/3;         //distance along X axis from the first triangle center to the section center
                                        Jx1=w1*qPow(h1,3)/36+w1*h1/2*qPow(yc1,2);           //moment of inertia about X axis of the first triangle
                                        Jy1=qPow(w1,3)*h1/36+w1*h1/2*qPow(xc1,2);           //moment of inertia about Y axis of the first triangle
                                    }
                                    else                                //point is on Xmax
                                    {
                                        w1=dXmax-p2.x();            //width of triangle between two points
                                        xc1=p1.x()-m_concreteCenter[i][j].x()-w1/3;         //distance along X axis from the first triangle center to the section center
                                        Jx1=-w1*qPow(h1,3)/36-w1*h1/2*qPow(yc1,2);           //moment of inertia about X axis of the first triangle
                                        Jy1=-qPow(w1,3)*h1/36-w1*h1/2*qPow(xc1,2);           //moment of inertia about Y axis of the first triangle
                                    }
                                    if (p1.y()>dYmin)                   //point is not on Ymin
                                    {
                                        h2=p1.y()-dYmin;                //height of the second triangle connected to face with Xmin
                                        yc2=m_concreteCenter[i][j].y()-p1.y()+h2/3;         //distance along Y axis from the second triangle center to the section center
                                        Jx2=w2*qPow(h2,3)/36+w2*h2/2*qPow(yc2,2);           //moment of inertia about X axis of the second triangle
                                        Jy2=qPow(w2,3)*h2/36+w2*h2/2*qPow(xc2,2);           //moment of inertia about Y axis of the second triangle
                                    }
                                    else                                //point is on Ymin
                                    {
                                        pYmin=m_dividedRegions[i][j][m_dividedFaces[i][j][0][0]-1];
                                        for (int p=1; p<m_dividedFaces[i][j][0].size(); ++p)
                                        {
                                            if(m_dividedRegions[i][j][m_dividedFaces[i][j][0][p]-1].y()<m_dividedRegions[i][j][m_dividedFaces[i][j][0][p-1]-1].y())
                                            {
                                                pYmin=m_dividedRegions[i][j][m_dividedFaces[i][j][0][p]-1];
                                            }
                                        }
                                        h2=pYmin.y()-p1.y();            //height of the second triangle connected to face with Xmin
                                        yc2=m_concreteCenter[i][j].y()-p1.y()-h2/3;         //distance along Y axis from the second triangle center to the section center
                                        Jx2=-w2*qPow(h2,3)/36-w2*h2/2*qPow(yc2,2);           //moment of inertia about X axis of the second triangle
                                        Jy2=-qPow(w2,3)*h2/36-w2*h2/2*qPow(xc2,2);           //moment of inertia about Y axis of the second triangle
                                    }
                                    if (p2.y()<dYmax)                   //point is not on Ymax
                                    {
                                        h3=dYmax-p2.y();                //height of the third triangle connected to face with Xmin
                                        yc3=p2.y()-m_concreteCenter[i][j].y()+h3/3;         //distance along Y axis from the third triangle center to the section center
                                        Jx3=w3*qPow(h3,3)/36+w3*h3/2*qPow(yc3,2);           //moment of inertia about X axis of the third triangle
                                        Jy3=qPow(w3,3)*h3/36+w3*h3/2*qPow(xc3,2);           //moment of inertia about Y axis of the third triangle
                                    }
                                    else                                //point is on Ymax
                                    {
                                        pYmax=m_dividedRegions[i][j][m_dividedFaces[i][j][0][0]-1];
                                        for (int p=1; p<m_dividedFaces[i][j][0].size(); ++p)
                                        {
                                            if(m_dividedRegions[i][j][m_dividedFaces[i][j][0][p]-1].y()>m_dividedRegions[i][j][m_dividedFaces[i][j][0][p-1]-1].y())
                                            {
                                                pYmax=m_dividedRegions[i][j][m_dividedFaces[i][j][0][p]-1];
                                            }
                                        }
                                        h3=dYmax-pYmin.y();            //height of the third triangle connected to face with Xmin
                                        yc3=p2.y()-m_concreteCenter[i][j].y()-h3/3;         //distance along Y axis from the third triangle center to the section center
                                        Jx3=-w3*qPow(h3,3)/36-w3*h3/2*qPow(yc3,2);           //moment of inertia about X axis of the third triangle
                                        Jy3=-qPow(w3,3)*h3/36-w3*h3/2*qPow(xc3,2);           //moment of inertia about Y axis of the third triangle
                                    }
                                }
                                else if (!freeFaces[1])         //face with connection to figure is with Ymin
                                {
                                    if (p1.x()>p2.x())
                                    {
                                        switchPoint(p1, p2);
                                    }
                                    w1=p2.x()-p1.x();             //width of triange between two points
                                    h2=p1.y()-dYmin;             //height of the second triangle connected to face with Ymin
                                    h3=p2.y()-dYmin;             //height of the third triangle connected to face with Ymin
                                    xc1=p2.x()-m_concreteCenter[i][j].x()-w1/3;       //distance along X axis from the first triangle center to the section center
                                    yc2=m_concreteCenter[i][j].y()-dYmin-h2/3;       //distance along Y axis from the second triangle center to the section center
                                    yc3=m_concreteCenter[i][j].y()-dYmin-h3/3;       //distance along Y axis from the third triangle center to the section center
                                    xc4=p2.x()-m_concreteCenter[i][j].x()-w1/2;       //distance along X axis from the rectangle center to the section center
                                    yc4=p1.y()-m_concreteCenter[i][j].y()-h2/2;       //distance along Y axis from the rectangle center to the section center
                                    Jy4=h2*qPow(w1,3)/12+h2*w1*qPow(xc4,2);          //moment of inertia about Y axis of the rectangle
                                    Jx4=qPow(h2,3)*w1/12+h2*w1*qPow(yc4,2);          //moment of inertia about X axis of the rectangle
                                    QPointF pXmin, pXmax;
                                    if (p1.y()<dYmax)                   //point is not on Ymax
                                    {
                                        h1=dYmax-p1.y();                //height of triangle between two points
                                        yc1=p1.y()-m_concreteCenter[i][j].y()+h1/3;         //distance along Y axis from the first triangle center to the section center
                                        Jx1=w1*qPow(h1,3)/36+w1*h1/2*qPow(yc1,2);           //moment of inertia about X axis of the first triangle
                                        Jy1=qPow(w1,3)*h1/36+w1*h1/2*qPow(xc1,2);           //moment of inertia about Y axis of the first triangle
                                    }
                                    else                                //point is on Ymax
                                    {
                                        h1=dYmax-p2.y();            //height of triangle between two points
                                        yc1=p1.y()-m_concreteCenter[i][j].y()-h1/3;         //distance along Y axis from the first triangle center to the section center
                                        Jx1=-w1*qPow(h1,3)/36-w1*h1/2*qPow(yc1,2);           //moment of inertia about X axis of the first triangle
                                        Jy1=-qPow(w1,3)*h1/36-w1*h1/2*qPow(xc1,2);           //moment of inertia about Y axis of the first triangle
                                    }
                                    if (p1.x()>dXmin)                   //point is not on Xmin
                                    {
                                        w2=p1.x()-dXmin;                //width of the second triangle connected to face with Ymin
                                        xc2=m_concreteCenter[i][j].x()-p1.x()+w2/3;         //distance along X axis from the second triangle center to the section center
                                        Jx2=w2*qPow(h2,3)/36+w2*h2/2*qPow(yc2,2);           //moment of inertia about X axis of the second triangle
                                        Jy2=qPow(w2,3)*h2/36+w2*h2/2*qPow(xc2,2);           //moment of inertia about Y axis of the second triangle
                                    }
                                    else                                //point is on Xmin
                                    {
                                        pXmin=m_dividedRegions[i][j][m_dividedFaces[i][j][1][0]-1];
                                        for (int p=1; p<m_dividedFaces[i][j][1].size(); ++p)
                                        {
                                            if(m_dividedRegions[i][j][m_dividedFaces[i][j][1][p]-1].x()<m_dividedRegions[i][j][m_dividedFaces[i][j][1][p-1]-1].x())
                                            {
                                                pXmin=m_dividedRegions[i][j][m_dividedFaces[i][j][1][p]-1];
                                            }
                                        }
                                        w2=pXmin.x()-p1.x();            //width of the second triangle connected to face with Ymin
                                        xc2=m_concreteCenter[i][j].x()-p1.x()-w2/3;         //distance along X axis from the second triangle center to the section center
                                        Jx2=-w2*qPow(h2,3)/36-w2*h2/2*qPow(yc2,2);           //moment of inertia about X axis of the second triangle
                                        Jy2=-qPow(w2,3)*h2/36-w2*h2/2*qPow(xc2,2);           //moment of inertia about Y axis of the second triangle
                                    }
                                    if (p2.x()<dXmax)                   //point is not on Xmax
                                    {
                                        w3=dXmax-p2.x();                //width of the third triangle connected to face with Ymin
                                        xc3=p2.x()-m_concreteCenter[i][j].x()+w3/3;         //distance along X axis from the third triangle center to the section center
                                        Jx3=w3*qPow(h3,3)/36+w3*h3/2*qPow(yc3,2);           //moment of inertia about X axis of the third triangle
                                        Jy3=qPow(w3,3)*h3/36+w3*h3/2*qPow(xc3,2);           //moment of inertia about Y axis of the third triangle
                                    }
                                    else                                //point is on Xmax
                                    {
                                        pXmax=m_dividedRegions[i][j][m_dividedFaces[i][j][1][0]-1];
                                        for (int p=1; p<m_dividedFaces[i][j][1].size(); ++p)
                                        {
                                            if(m_dividedRegions[i][j][m_dividedFaces[i][j][1][p]-1].x()>m_dividedRegions[i][j][m_dividedFaces[i][j][1][p-1]-1].x())
                                            {
                                                pXmax=m_dividedRegions[i][j][m_dividedFaces[i][j][1][p]-1];
                                            }
                                        }
                                        w3=dXmax-pXmin.x();            //width of the third triangle connected to face with Ymin
                                        xc3=p2.x()-m_concreteCenter[i][j].x()-w3/3;         //distance along X axis from the third triangle center to the section center
                                        Jx3=-w3*qPow(h3,3)/36-w3*h3/2*qPow(yc3,2);           //moment of inertia about X axis of the third triangle
                                        Jy3=-qPow(w3,3)*h3/36-w3*h3/2*qPow(xc3,2);           //moment of inertia about Y axis of the third triangle
                                    }
                                }
                                else if (!freeFaces[2])         //face with connection to figure is with Xmax
                                {
                                    if (p1.y()>p2.y())
                                    {
                                        switchPoint(p1, p2);
                                    }
                                    h1=p2.y()-p1.y();             //height of the first triange between two points
                                    w2=dXmax-p1.x();             //width of the second triangle connected to face with Xmax
                                    w3=dXmax-p2.x();             //width of the third triangle connected to face with Xmax
                                    yc1=p2.y()-m_concreteCenter[i][j].y()-h1/3;       //distance along Y axis from the first triangle center to the section center
                                    xc2=dXmax-m_concreteCenter[i][j].x()-w2/3;       //distance along X axis from the second triangle center to the section center
                                    xc3=dXmax-m_concreteCenter[i][j].x()-w3/3;       //distance along X axis from the third triangle center to the section center
                                    yc4=p2.y()-m_concreteCenter[i][j].y()-h1/2;       //distance along Y axis from the rectangle center to the section center
                                    xc4=dXmax-m_concreteCenter[i][j].x()-w2/2;       //distance along X axis from the rectangle center to the section center
                                    Jx4=w2*qPow(h1,3)/12+w2*h1*qPow(yc4,2);          //moment of inertia about X axis of the rectangle
                                    Jy4=qPow(w2,3)*h1/12+w2*h1*qPow(xc4,2);          //moment of inertia about Y axis of the rectangle
                                    QPointF pYmin, pYmax;
                                    if (p1.x()>dXmin)                   //point is not on Xmin
                                    {
                                        w1=p1.x()-dXmin;                //width of triangle between two points
                                        xc1=m_concreteCenter[i][j].x()-p1.x()+w1/3;         //distance along X axis from the first triangle center to the section center
                                        Jx1=w1*qPow(h1,3)/36+w1*h1/2*qPow(yc1,2);           //moment of inertia about X axis of the first triangle
                                        Jy1=qPow(w1,3)*h1/36+w1*h1/2*qPow(xc1,2);           //moment of inertia about Y axis of the first triangle
                                    }
                                    else                                //point is on Xmin
                                    {
                                        w1=p2.x()-dXmin;            //width of triangle between two points
                                        xc1=m_concreteCenter[i][j].x()-p1.x()-w1/3;         //distance along X axis from the first triangle center to the section center
                                        Jx1=-w1*qPow(h1,3)/36-w1*h1/2*qPow(yc1,2);           //moment of inertia about X axis of the first triangle
                                        Jy1=-qPow(w1,3)*h1/36-w1*h1/2*qPow(xc1,2);           //moment of inertia about Y axis of the first triangle
                                    }
                                    if (p1.y()>dYmin)                   //point is not on Ymin
                                    {
                                        h2=p1.y()-dYmin;                //height of the second triangle connected to face with Xmax
                                        yc2=m_concreteCenter[i][j].y()-p1.y()+h2/3;         //distance along Y axis from the second triangle center to the section center
                                        Jx2=w2*qPow(h2,3)/36+w2*h2/2*qPow(yc2,2);           //moment of inertia about X axis of the second triangle
                                        Jy2=qPow(w2,3)*h2/36+w2*h2/2*qPow(xc2,2);           //moment of inertia about Y axis of the second triangle
                                    }
                                    else                                //point is on Ymin
                                    {
                                        pYmin=m_dividedRegions[i][j][m_dividedFaces[i][j][2][0]-1];
                                        for (int p=1; p<m_dividedFaces[i][j][2].size(); ++p)
                                        {
                                            if(m_dividedRegions[i][j][m_dividedFaces[i][j][2][p]-1].y()<m_dividedRegions[i][j][m_dividedFaces[i][j][2][p-1]-1].y())
                                            {
                                                pYmin=m_dividedRegions[i][j][m_dividedFaces[i][j][2][p]-1];
                                            }
                                        }
                                        h2=pYmin.y()-p1.y();            //height of the second triangle connected to face with Xmax
                                        yc2=m_concreteCenter[i][j].y()-p1.y()-h2/3;         //distance along Y axis from the second triangle center to the section center
                                        Jx2=-w2*qPow(h2,3)/36-w2*h2/2*qPow(yc2,2);           //moment of inertia about X axis of the second triangle
                                        Jy2=-qPow(w2,3)*h2/36-w2*h2/2*qPow(xc2,2);           //moment of inertia about Y axis of the second triangle
                                    }
                                    if (p2.y()<dYmax)                   //point is not on Ymax
                                    {
                                        h3=dYmax-p2.y();                //height of the third triangle connected to face with Xmax
                                        yc3=p2.y()-m_concreteCenter[i][j].y()+h3/3;         //distance along Y axis from the third triangle center to the section center
                                        Jx3=w3*qPow(h3,3)/36+w3*h3/2*qPow(yc3,2);           //moment of inertia about X axis of the third triangle
                                        Jy3=qPow(w3,3)*h3/36+w3*h3/2*qPow(xc3,2);           //moment of inertia about Y axis of the third triangle
                                    }
                                    else                                //point is on Ymax
                                    {
                                        pYmax=m_dividedRegions[i][j][m_dividedFaces[i][j][2][0]-1];
                                        for (int p=1; p<m_dividedFaces[i][j][2].size(); ++p)
                                        {
                                            if(m_dividedRegions[i][j][m_dividedFaces[i][j][2][p]-1].y()>m_dividedRegions[i][j][m_dividedFaces[i][j][2][p-1]-1].y())
                                            {
                                                pYmax=m_dividedRegions[i][j][m_dividedFaces[i][j][2][p]-1];
                                            }
                                        }
                                        h3=dYmax-pYmin.y();            //height of the third triangle connected to face with Xmax
                                        yc3=p2.y()-m_concreteCenter[i][j].y()-h3/3;         //distance along Y axis from the third triangle center to the section center
                                        Jx3=-w3*qPow(h3,3)/36-w3*h3/2*qPow(yc3,2);           //moment of inertia about X axis of the third triangle
                                        Jy3=-qPow(w3,3)*h3/36-w3*h3/2*qPow(xc3,2);           //moment of inertia about Y axis of the third triangle
                                    }
                                }
                                else //if (!freeFaces[3])         //face with connection to figure is with Ymax
                                {
                                    if (p1.x()>p2.x())
                                    {
                                        switchPoint(p1, p2);
                                    }
                                    w1=p2.x()-p1.x();             //width of triange between two points
                                    h2=dYmax-p1.y();             //height of the second triangle connected to face with Ymax
                                    h3=dYmax-p2.y();             //height of the third triangle connected to face with Ymax
                                    xc1=p2.x()-m_concreteCenter[i][j].x()-w1/3;       //distance along X axis from the first triangle center to the section center
                                    yc2=dYmax-m_concreteCenter[i][j].y()-h2/3;       //distance along Y axis from the second triangle center to the section center
                                    yc3=dYmax-m_concreteCenter[i][j].y()-h3/3;       //distance along Y axis from the third triangle center to the section center
                                    xc4=p2.x()-m_concreteCenter[i][j].x()-w1/2;       //distance along X axis from the rectangle center to the section center
                                    yc4=dYmax-m_concreteCenter[i][j].y()-h2/2;       //distance along Y axis from the rectangle center to the section center
                                    Jy4=h2*qPow(w1,3)/12+h2*w1*qPow(xc4,2);          //moment of inertia about Y axis of the rectangle
                                    Jx4=qPow(h2,3)*w1/12+h2*w1*qPow(yc4,2);          //moment of inertia about X axis of the rectangle
                                    QPointF pXmin, pXmax;
                                    if (p1.y()>dYmin)                   //point is not on Ymin
                                    {
                                        h1=p1.y()-dYmin;                //height of triangle between two points
                                        yc1=m_concreteCenter[i][j].y()-p1.y()+h1/3;         //distance along Y axis from the first triangle center to the section center
                                        Jx1=w1*qPow(h1,3)/36+w1*h1/2*qPow(yc1,2);           //moment of inertia about X axis of the first triangle
                                        Jy1=qPow(w1,3)*h1/36+w1*h1/2*qPow(xc1,2);           //moment of inertia about Y axis of the first triangle
                                    }
                                    else                                //point is on Ymin
                                    {
                                        h1=p2.y()-dYmin;            //height of triangle between two points
                                        yc1=m_concreteCenter[i][j].y()-p1.y()-h1/3;         //distance along Y axis from the first triangle center to the section center
                                        Jx1=-w1*qPow(h1,3)/36-w1*h1/2*qPow(yc1,2);           //moment of inertia about X axis of the first triangle
                                        Jy1=-qPow(w1,3)*h1/36-w1*h1/2*qPow(xc1,2);           //moment of inertia about Y axis of the first triangle
                                    }
                                    if (p1.x()>dXmin)                   //point is not on Xmin
                                    {
                                        w2=p1.x()-dXmin;                //width of the second triangle connected to face with Ymax
                                        xc2=m_concreteCenter[i][j].x()-p1.x()+w2/3;         //distance along X axis from the second triangle center to the section center
                                        Jx2=w2*qPow(h2,3)/36+w2*h2/2*qPow(yc2,2);           //moment of inertia about X axis of the second triangle
                                        Jy2=qPow(w2,3)*h2/36+w2*h2/2*qPow(xc2,2);           //moment of inertia about Y axis of the second triangle
                                    }
                                    else                                //point is on Xmin
                                    {
                                        pXmin=m_dividedRegions[i][j][m_dividedFaces[i][j][3][0]-1];
                                        for (int p=1; p<m_dividedFaces[i][j][3].size(); ++p)
                                        {
                                            if(m_dividedRegions[i][j][m_dividedFaces[i][j][3][p]-1].x()<m_dividedRegions[i][j][m_dividedFaces[i][j][3][p-1]-1].x())
                                            {
                                                pXmin=m_dividedRegions[i][j][m_dividedFaces[i][j][3][p]-1];
                                            }
                                        }
                                        w2=pXmin.x()-p1.x();            //width of the second triangle connected to face with Ymax
                                        xc2=m_concreteCenter[i][j].x()-p1.x()-w2/3;         //distance along X axis from the second triangle center to the section center
                                        Jx2=-w2*qPow(h2,3)/36-w2*h2/2*qPow(yc2,2);           //moment of inertia about X axis of the second triangle
                                        Jy2=-qPow(w2,3)*h2/36-w2*h2/2*qPow(xc2,2);           //moment of inertia about Y axis of the second triangle
                                    }
                                    if (p2.x()<dXmax)                   //point is not on Xmax
                                    {
                                        w3=dXmax-p2.x();                //width of the third triangle connected to face with Ymax
                                        xc3=p2.x()-m_concreteCenter[i][j].x()+w3/3;         //distance along X axis from the third triangle center to the section center
                                        Jx3=w3*qPow(h3,3)/36+w3*h3/2*qPow(yc3,2);           //moment of inertia about X axis of the third triangle
                                        Jy3=qPow(w3,3)*h3/36+w3*h3/2*qPow(xc3,2);           //moment of inertia about Y axis of the third triangle
                                    }
                                    else                                //point is on Xmax
                                    {
                                        pXmax=m_dividedRegions[i][j][m_dividedFaces[i][j][3][0]-1];
                                        for (int p=1; p<m_dividedFaces[i][j][3].size(); ++p)
                                        {
                                            if(m_dividedRegions[i][j][m_dividedFaces[i][j][3][p]-1].x()>m_dividedRegions[i][j][m_dividedFaces[i][j][3][p-1]-1].x())
                                            {
                                                pXmax=m_dividedRegions[i][j][m_dividedFaces[i][j][3][p]-1];
                                            }
                                        }
                                        w3=dXmax-pXmin.x();            //width of the third triangle connected to face with Ymax
                                        xc3=p2.x()-m_concreteCenter[i][j].x()-w3/3;         //distance along X axis from the third triangle center to the section center
                                        Jx3=-w3*qPow(h3,3)/36-w3*h3/2*qPow(yc3,2);           //moment of inertia about X axis of the third triangle
                                        Jy3=-qPow(w3,3)*h3/36-w3*h3/2*qPow(xc3,2);           //moment of inertia about Y axis of the third triangle
                                    }
                                }
                                m_concreteJx[i][j]=Jx1+Jx2+Jx3+Jx4;
                                m_concreteJy[i][j]=Jy1+Jy2+Jy3+Jy4;
                                JxIsSet=true;
                            }
                            break;
                        }
                    }
                    else
                    {
                        m_concreteArea[i][j]=0;
                        m_concreteJx[i][j]=0;
                        m_concreteJy[i][j]=0;
                    }

                }
            }
        }
        myCalc->setConcreteArea(m_concreteArea);
        myCalc->setConcreteCenter(m_concreteCenter);
        myCalc->setReinfArea(m_reinfCircles);
        myCalc->setCenterPoint();
        myCalc->setMomentsOfInertia(m_concreteJx, m_concreteJy);
        myCalc->calculate();
    }
}

void Scene::slotFitView()
{
    this->setSceneRect(lowX-m_viewMargin,lowY-m_viewMargin,m_recWidth+m_viewMargin, m_recHeight+m_viewMargin);
    //emit signalFitView();
}

void Scene::slotSetN(QString str)
{
    QRegularExpression reg("[+-]?([0-9]*[.|,])?[0-9]+");
    QRegularExpressionMatchIterator it=reg.globalMatch(str);
    if (it.hasNext())
    {
        QRegularExpressionMatch match=it.next();
        qDebug()<<"filtered N="<<match.captured(0);
        myCalc->setN(QString(match.captured(0)).toDouble());
        m_NIsSet=true;
        checkForces();
    }
    else {
        m_NIsSet=false;
    }
    qDebug()<<"NIsSet="<<m_NIsSet;
}

void Scene::slotSetMx(QString str)
{
    QRegularExpression reg("[+-]?([0-9]*[.|,])?[0-9]+");
    QRegularExpressionMatchIterator it=reg.globalMatch(str);
    if (it.hasNext())
    {
        QRegularExpressionMatch match=it.next();
        qDebug()<<"filtered Mx="<<match.captured(0);
        myCalc->setMx(QString(match.captured(0)).toDouble());
        m_MxIsSet=true;
        checkForces();
    }
    else {
        m_MxIsSet=false;
    }
    qDebug()<<"MxIsSet="<<m_MxIsSet;
}

void Scene::slotSetMy(QString str)
{
    QRegularExpression reg("[+-]?([0-9]*[.|,])?[0-9]+");
    QRegularExpressionMatchIterator it=reg.globalMatch(str);
    if (it.hasNext())
    {
        QRegularExpressionMatch match=it.next();
        qDebug()<<"filtered My="<<match.captured(0);
        myCalc->setMy(QString(match.captured(0)).toDouble());
        m_MyIsSet=true;
        checkForces();
    }
    else {
        m_MyIsSet=false;
    }
    qDebug()<<"MyIsSet="<<m_MyIsSet;
}

void Scene::slotSetEb(double d)
{
    emit signalSetEb(d);
}

void Scene::slotSetEs(double d)
{
    emit signalSetEs(d);
}

void Scene::slotSetRb(double d)
{
    emit signalSetRb(d);
}

void Scene::slotSetRbt(double d)
{
    emit signalSetRbt(d);
}

void Scene::slotSetRs(double d)
{
    emit signalSetRs(d);
}

void Scene::slotImportPoints()
{
    myExcel=new ExcelInOutHelper(this);
    myExcel->importPoints(QFileDialog::getOpenFileName(nullptr, "Open file with data", "data.xls", "excel(*.xls *.xlsx)"));
    setDrawLine();
    for (int i=0; i<myExcel->getConcreteData().size(); ++i)
    {
        drawPoint(myExcel->getConcreteData()[i]);
        emit signalPointAdded(toSceneCoord(myExcel->getConcreteData()[i]));
    }
    slotGetCommand("c");
    setDrawPoint();
    for (int i=0; i<myExcel->getReinfData().size(); ++i)
    {
        slotSetRDiameter(myExcel->getReinfData()[i].first);
        drawPoint(myExcel->getReinfData()[i].second);
        emit signalPointAdded(toSceneCoord(myExcel->getReinfData()[i].second));
    }
    slotGetCommand("d");
}

void Scene::slotExportPoints()
{
    myExcel=new ExcelInOutHelper(this);
//    QString fileName=QFileDialog::getSaveFileName(this, "Save file with data", "data.xls", "excel(*.xls *.xlsx)");
//    qDebug()<<fileName;
    if (m_doneConcretePath&&m_doneReinforcement)
    {
        myExcel->exportPoints(m_concretePoints, m_reinfCircles);
    }
    else
    {
       if (m_doneConcretePath)
       {
           int msgBox=QMessageBox::warning(nullptr,"Warning","Draw Reinforcement first");
       }
       else
       {
           int msgBox=QMessageBox::warning(nullptr,"Warning","Draw Concrete section first");
       }
    }
}

void Scene::slotCalcStart()
{
    emit signalCalcStart();
}

void Scene::slotPercentChanged(int i)
{
    emit signalPercentChanged(i);
}

void Scene::slotCalcEnd(bool b)
{
    emit signalCalcEnd(b);
}

void Scene::slotDrawStress()
{
    QString outputTitle="Stress in MPa";
    QFont titleFont=QFont("Helvetica", 20, QFont::Normal);
    m_resultTitle=this->addSimpleText(outputTitle, titleFont);
    m_resultTitle->setPos(lowX+m_recWidth/2-100,lowY-40);
    m_concreteResultText.fill(QVector<QGraphicsSimpleTextItem*>(),nXdivisions);
    for (int i=0; i<m_concreteCenter.size(); ++i)
    {
        m_concreteResultText[i].fill(nullptr,nYdivisions);
        for (int j=0; j<m_concreteCenter[i].size(); ++j)
        {
            m_concreteResultText[i][j]=this->addSimpleText(QString::number(myCalc->getCStress()[i][j]/1000,'f',1));
            m_concreteResultText[i][j]->setPos(m_concreteCenter[i][j]);
            if (m_isRect)
            {
                m_divisionItems[i][j]->setZValue(-1);
                m_divisionItems[i][j]->setBrush(myCalc->getCStress()[i][j]==0?bTensile:bCompressed);

            }
            else
            {
                m_divisionPaths[i][j]->setZValue(-1);
                m_divisionPaths[i][j]->setBrush(myCalc->getCStress()[i][j]==0?bTensile:bCompressed);

            }
        }
    }
    m_reinfResultText.fill(nullptr,m_reinfCircles.size());
    for (int i=0;i<m_reinfCircles.size(); ++i)
    {
        m_reinfResultText[i]=this->addSimpleText(QString::number(myCalc->getRStress()[i]/1000,'f',1));
        m_reinfResultText[i]->setPos(m_reinfCircles[i].second);
    }
}

void Scene::DrawStrain()
{
    QString outputTitle="Strain x1000";
    QFont titleFont=QFont("Helvetica", 20, QFont::Normal);
    this->removeItem(m_resultTitle);
    m_resultTitle=this->addSimpleText(outputTitle, titleFont);
    m_resultTitle->setPos(lowX+m_recWidth/2-100,lowY-40);
    for (int i=0; i<m_concreteCenter.size(); ++i)
    {
        for (int j=0; j<m_concreteCenter[i].size(); ++j)
        {
            this->removeItem(m_concreteResultText[i][j]);
            m_concreteResultText[i][j]=this->addSimpleText(QString::number(myCalc->getCStrain()[i][j]*1000,'f',4));
            m_concreteResultText[i][j]->setPos(m_concreteCenter[i][j]);
        }
    }
    for (int i=0;i<m_reinfCircles.size(); ++i)
    {
        this->removeItem(m_reinfResultText[i]);
        m_reinfResultText[i]=this->addSimpleText(QString::number(myCalc->getRStrain()[i]*1000,'f',4));
        m_reinfResultText[i]->setPos(m_reinfCircles[i].second);
    }
}

void Scene::DrawArea()
{
    QString outputTitle="Area in sm2";
    QFont titleFont=QFont("Helvetica", 20, QFont::Normal);
    this->removeItem(m_resultTitle);
    m_resultTitle=this->addSimpleText(outputTitle, titleFont);
    m_resultTitle->setPos(lowX+m_recWidth/2-100,lowY-40);
    for (int i=0; i<m_concreteCenter.size(); ++i)
    {
        for (int j=0; j<m_concreteCenter[i].size(); ++j)
        {
            this->removeItem(m_concreteResultText[i][j]);
            m_concreteResultText[i][j]=this->addSimpleText(QString::number(myCalc->getCArea()[i][j]*10000,'f',1));
            m_concreteResultText[i][j]->setPos(m_concreteCenter[i][j]);
        }
    }
    for (int i=0;i<m_reinfCircles.size(); ++i)
    {
        this->removeItem(m_reinfResultText[i]);
        m_reinfResultText[i]=this->addSimpleText(QString::number(myCalc->getRArea()[i]*10000,'f',1));
        m_reinfResultText[i]->setPos(m_reinfCircles[i].second);
    }
}

void Scene::slotExportStart()
{
    emit signalExportStart();
}

void Scene::slotExportPercentChanged(int i)
{
    emit signalExportPercentChanged(i);
}

void Scene::slotExportEnd()
{
    emit signalExportEnd();
}

void Scene::slotApplyPressed(int i)
{
    if (!m_resultIsSaved)
    {
        myCalc->saveResult();
        m_resultIsSaved=true;
    }
    if(i!=m_resultMode)
    {
        m_resultMode=i;
        switch (i)
        {
        case 1:
            DrawStress();
            break;
        case 2:
            DrawStress();
            break;
        case 3:
            DrawStrain();
            break;
        case 4:
            DrawArea();
            break;
        }
    }
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
            //brush.setColor(Qt::yellow);
            if (m_pathItem!=nullptr)
            {
                this->removeItem(m_pathItem);
            }
            m_pathItem=this->addPath(*m_concretePath, pen, QBrush(QColor(0,180,220,100)));
            m_pathItem->setZValue(-2);
            emit signalDrawMode(false);
            emit signalSectDone(true);
            getSectSizes();
        }
    }
    if (m_drawMode==POINT)
    {
        if (str=="d"||str=="D")
        {
            qDebug()<<"draw mode is off";
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
         emit signalPointAdded(p);
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
        m_dividedFaces.fill(QVector<QVector<QVector<int>>>(),nXdivisions);
        for (int i=1; i<=nXdivisions; ++i)
        {
            m_dividedRegions[i-1].fill(QVector<QPointF>(),nYdivisions);
            m_dividedFaces[i-1].fill(QVector<QVector<int>>(),nYdivisions);
            for (int j=1; j<=nYdivisions; ++j)
            {
                m_dividedFaces[i-1][j-1].fill(QVector<int>(),4);
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
                if(lowInterX1.y()>y1&&lowInterX1.y()<y2)                                        //low intersection is on the line with x=x1
                {
                    m_dividedRegions[i-1][j-1].append(QPointF(x1,lowInterX1.y()));
                    m_dividedFaces[i-1][j-1][0].append(m_dividedRegions[i-1][j-1].size());
                }
                if(highInterX1.y()>y1&&highInterX1.y()<y2&&lowInterX1.y()!=highInterX1.y())     //high intersection is on the line with x=x1
                {
                    m_dividedRegions[i-1][j-1].append(QPointF(x1,highInterX1.y()));
                    m_dividedFaces[i-1][j-1][0].append(m_dividedRegions[i-1][j-1].size());
                }
                if((lowInterX1.y()<=y1&&highInterX1.y()>=y1)&&(lowInterY1.x()<=x1&&highInterY1.x()>=x1))   //point x1,y1 is inside the figure
                {
                    m_dividedRegions[i-1][j-1].append(QPointF(x1,y1));
                    m_dividedFaces[i-1][j-1][0].append(m_dividedRegions[i-1][j-1].size());
                    m_dividedFaces[i-1][j-1][1].append(m_dividedRegions[i-1][j-1].size());
                }
                if(lowInterY1.x()>x1&&lowInterY1.x()<x2)                                            //low intersection is on the line with y=y1
                {
                    m_dividedRegions[i-1][j-1].append(QPointF(lowInterY1.x(),y1));
                    m_dividedFaces[i-1][j-1][1].append(m_dividedRegions[i-1][j-1].size());
                }
                if(highInterY1.x()>x1&&highInterY1.x()<x2&&lowInterY1.x()!=highInterY1.x())         //high intersection is on the line with y=y1
                {
                    m_dividedRegions[i-1][j-1].append(QPointF(highInterY1.x(),y1));
                    m_dividedFaces[i-1][j-1][1].append(m_dividedRegions[i-1][j-1].size());
                }
                if((lowInterX2.y()<=y1&&highInterX2.y()>=y1)&&(lowInterY1.x()<=x2&&highInterY1.x()>=x2))   //point x2,y1 is inside the figure
                {
                    m_dividedRegions[i-1][j-1].append(QPointF(x2,y1));
                    m_dividedFaces[i-1][j-1][1].append(m_dividedRegions[i-1][j-1].size());
                    m_dividedFaces[i-1][j-1][2].append(m_dividedRegions[i-1][j-1].size());
                }
                if(lowInterX2.y()>y1&&lowInterX2.y()<y2)                                          //low intersection is on the line with x=x2
                {
                    m_dividedRegions[i-1][j-1].append(QPointF(x2,lowInterX2.y()));
                    m_dividedFaces[i-1][j-1][2].append(m_dividedRegions[i-1][j-1].size());
                }
                if(highInterX2.y()>y1&&highInterX2.y()<y2&&lowInterX2.y()!=highInterX2.y())       //high intersection is on the line with x=x2
                {
                    m_dividedRegions[i-1][j-1].append(QPointF(x2,highInterX2.y()));
                    m_dividedFaces[i-1][j-1][2].append(m_dividedRegions[i-1][j-1].size());
                }
                if((lowInterX2.y()<=y2&&highInterX2.y()>=y2)&&(lowInterY2.x()<=x2&&highInterY2.x()>=x2))   //point x2,y2 is inside the figure
                {
                    m_dividedRegions[i-1][j-1].append(QPointF(x2,y2));
                    m_dividedFaces[i-1][j-1][2].append(m_dividedRegions[i-1][j-1].size());
                    m_dividedFaces[i-1][j-1][3].append(m_dividedRegions[i-1][j-1].size());
                }
                if(lowInterY2.x()>x1&&lowInterY2.x()<x2)                                            //low intersection is on the line with y=y2
                {
                    m_dividedRegions[i-1][j-1].append(QPointF(lowInterY2.x(),y2));
                    m_dividedFaces[i-1][j-1][3].append(m_dividedRegions[i-1][j-1].size());
                }
                if(highInterY2.x()>x1&&highInterY2.x()<x2&&lowInterY2.x()!=highInterY2.x())         //high intersection is on the line with y=y2
                {
                    m_dividedRegions[i-1][j-1].append(QPointF(highInterY2.x(),y2));
                    m_dividedFaces[i-1][j-1][3].append(m_dividedRegions[i-1][j-1].size());
                }
                if((lowInterX1.y()<=y2&&highInterX1.y()>=y2)&&(lowInterY2.x()<=x1&&highInterY2.x()>=x1))   //point x1,y2 is inside the figure
                {
                    m_dividedRegions[i-1][j-1].append(QPointF(x1,y2));
                    m_dividedFaces[i-1][j-1][3].append(m_dividedRegions[i-1][j-1].size());
                    m_dividedFaces[i-1][j-1][0].append(m_dividedRegions[i-1][j-1].size());
                }
                for (int k=0; k<m_concretePoints.size(); ++k)
                {
                    if ((m_concretePoints[k].x()>x1&&m_concretePoints[k].x()<x2)&&(m_concretePoints[k].y()>y1&&m_concretePoints[k].y()<y2))
                    {
                        m_dividedRegions[i-1][j-1].append(QPointF(m_concretePoints[k].x(),m_concretePoints[k].y()));
                        int num=m_dividedRegions[i-1][j-1].size()-1;
                        if(!isBetween(m_dividedRegions[i-1][j-1][num],m_dividedRegions[i-1][j-1][0],m_dividedRegions[i-1][j-1][num-1]))
                        {
                            QPointF curPoint=m_dividedRegions[i-1][j-1][num];       //if the point is not in the right place
                            int l=0;
                            do
                            {
                                for (int f=0; f<4; ++f)
                                {
                                    for (int g=0; g<m_dividedFaces[i-1][j-1][f].size();++g)
                                    {
                                        if(m_dividedFaces[i-1][j-1][f][g]==num-l)
                                        {m_dividedFaces[i-1][j-1][f][g]=num-l+1;}
                                    }
                                }
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
                for (int f=0; f<4; ++f)
                {
                    if (!m_dividedFaces[i-1][j-1][f].empty())
                    {
                        for (int g=0; g<m_dividedFaces[i-1][j-1][f].size();++g)
                        {
                            qDebug()<<"face №"+QString::number(f)+" have intersection in x:"+QString::number(m_dividedRegions[i-1][j-1][m_dividedFaces[i-1][j-1][f][g]-1].x())+" y:"+QString::number(m_dividedRegions[i-1][j-1][m_dividedFaces[i-1][j-1][f][g]-1].y());
                        }
                    }
                }
            }
        }
    }
    drawDivisions();
}

void Scene::checkForces()
{
    if(m_NIsSet&&m_MxIsSet&&m_MyIsSet)
    {emit signalForcesDone(true);}
}

void Scene::switchPoint(QPointF& p1, QPointF& p2)
{
    QPointF temp;
    temp.setX(p1.x());
    temp.setY(p1.y());
    p1.setX(p2.x());
    p1.setY(p2.y());
    p2.setX(temp.x());
    p2.setY(temp.y());
}

void Scene::DrawStress()
{
    QString outputTitle;
    if (m_resultMode==1)
    {
        outputTitle="Stress in MPa";
    }
    else
    {
        outputTitle="Stress in kg/sm2";
    }
    QFont titleFont=QFont("Helvetica", 20, QFont::Normal);
    this->removeItem(m_resultTitle);
    m_resultTitle=this->addSimpleText(outputTitle, titleFont);
    m_resultTitle->setPos(lowX+m_recWidth/2-100,lowY-40);
    QString outValue;
    for (int i=0; i<m_concreteCenter.size(); ++i)
    {
        for (int j=0; j<m_concreteCenter[i].size(); ++j)
        {
            this->removeItem(m_concreteResultText[i][j]);
            m_resultMode==1?outValue=QString::number(myCalc->getCStress()[i][j]/1000,'f',1):outValue=QString::number(myCalc->getCStress()[i][j]/98,'f',1);
            m_concreteResultText[i][j]=this->addSimpleText(outValue);
            m_concreteResultText[i][j]->setPos(m_concreteCenter[i][j]);
        }
    }
    for (int i=0;i<m_reinfCircles.size(); ++i)
    {
        this->removeItem(m_reinfResultText[i]);
        m_resultMode==1?outValue=QString::number(myCalc->getRStress()[i]/1000,'f',1):outValue=QString::number(myCalc->getRStress()[i]/98,'f',1);
        m_reinfResultText[i]=this->addSimpleText(outValue);
        m_reinfResultText[i]->setPos(m_reinfCircles[i].second);
    }
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
    m_dividedPoints.erase(m_dividedPoints.begin(),m_dividedPoints.end());
    m_dividedRegions.erase(m_dividedRegions.begin(),m_dividedRegions.end());
    m_dividedFaces.erase(m_dividedFaces.begin(),m_dividedFaces.end());
    m_reinfCircles.erase(m_reinfCircles.begin(),m_reinfCircles.end());
    m_doneConcretePath=false;
    m_doneReinforcement=false;
    m_sectDivided=false;
    delete m_concretePath;
    m_concretePath=new QPainterPath();
    m_pathItem=nullptr;
    delete m_currentItem;
    m_pointsItems.erase(m_pointsItems.begin(),m_pointsItems.end());
    m_divisionItems.erase(m_divisionItems.begin(),m_divisionItems.end());
    m_divisionPaths.erase(m_divisionPaths.begin(),m_divisionPaths.end());
    m_reinfItems.erase(m_reinfItems.begin(),m_reinfItems.end());
    m_concreteArea.erase(m_concreteArea.begin(),m_concreteArea.end());
    m_concreteJx.erase(m_concreteJx.begin(),m_concreteJx.end());
    m_concreteJy.erase(m_concreteJy.begin(),m_concreteJy.end());
    m_concreteCenter.erase(m_concreteCenter.begin(),m_concreteCenter.end());
    m_concreteResultText.erase(m_concreteResultText.begin(),m_concreteResultText.end());
    m_reinfResultText.erase(m_reinfResultText.begin(),m_reinfResultText.end());
    m_recWidth=0;
    m_recHeight=0;
    lowX=0;
    highX=0;
    lowY=0;
    highY=0;
    emit signalSceneCleared(true);
    emit signalReinfCleared(true);
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
            if (m_pathItem!=nullptr)
            {
                this->removeItem(m_pathItem);
            }
            m_pathItem=this->addPath(*m_concretePath, pen, QBrush(QColor(0,180,220,100)));
            m_pathItem->setZValue(-2);
        }
    }
    if (m_drawMode!=NONE)
    {
        if (m_drawMode==POINT)
        {   //we are drawing a point as a reinforcement bar
            pen.setBrush(QBrush(Qt::black));
            m_currentItem=this->addEllipse(point.x()-m_currDiam/2, point.y()-m_currDiam/2,m_currDiam,m_currDiam,pen,QBrush(QColor(180,0,220,100)));
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
        m_divisionItems.fill(QVector<QGraphicsRectItem*>(),nXdivisions);
        for (int i=1; i<=nXdivisions; ++i)
        {
            m_divisionItems[i].fill(nullptr,nYdivisions);
            for (int j=1; j<=nYdivisions; ++j)
            {
                QRectF r=QRectF(m_dividedPoints[i-1][j-1].x(),m_dividedPoints[i-1][j-1].y(),(m_dividedPoints[i][j].x()-m_dividedPoints[i-1][j].x()),(m_dividedPoints[i][j].y()-m_dividedPoints[i][j-1].y()));
                m_divisionItems[i][j]=this->addRect(r,pen,brush);
                qDebug()<<"x:"+QString::number(m_dividedPoints[i][j].x())+" y:"+ QString::number(m_dividedPoints[i][j].y());
            }
        }
    }
    else                //draw custom polygon
    {
        m_divisionPaths.fill(QVector<QGraphicsPathItem*>(),nXdivisions);
        for (int i=0; i<nXdivisions; ++i)
        {
            m_divisionPaths[i].fill(nullptr,nYdivisions);
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
                    m_divisionPaths[i][j]=this->addPath(p,pen,brush);
                }
            }
        }
    }
    qDebug()<<"Draw divisions is finished";
    emit signalSceneDivided(true);
    m_sectDivided=true;
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
