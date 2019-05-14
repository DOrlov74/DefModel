#include "calculation.h"
#include <QDebug>
#include <QPointF>
#include <QVector>
#include <QtMath>

Calculation::Calculation(QObject *parent) : QObject(parent)
{

}

void Calculation::setXdivision(uint a)
{
    nXdivisions=a;
}

void Calculation::setYdivision(uint b)
{
    nYdivisions=b;
}

void Calculation::setConcreteArea(const QVector<QVector<double>>& vArea)
{
    m_concreteArea.fill(QVector<double>(),nXdivisions);
    for (int i=0; i<vArea.size();++i)
    {
        m_concreteArea[i].fill(0,nYdivisions);
        for  (int j=0; j<vArea[i].size(); ++j)
        {
            m_concreteArea[i][j]=vArea[i][j];
            qDebug()<<"Area of concrete part in row:"+QString::number(i+1)+" in column:"+QString::number(j+1)+" is:"+QString::number(m_concreteArea[i][j]);
        }
    }
}

void Calculation::setConcreteCenter(const QVector<QVector<QPointF>>& vCenter)
{
    m_concreteCenter.fill(QVector<QPointF>(),nXdivisions);
    for (int i=0; i<vCenter.size();++i)
    {
        m_concreteCenter[i].fill(QPointF(0,0),nYdivisions);
        for  (int j=0; j<vCenter[i].size(); ++j)
        {
            m_concreteCenter[i][j].setX(vCenter[i][j].x());
            m_concreteCenter[i][j].setY(vCenter[i][j].y());
            qDebug()<<"Center point of concrete part in row:"+QString::number(i+1)+" in column:"+QString::number(j+1)+" is at x:"+QString::number(m_concreteCenter[i][j].x())+" y:"+QString::number(m_concreteCenter[i][j].y());
        }
    }
}

void Calculation::setReinfArea(const QVector<QPair<uint, QPointF>>& vReinf)
{
    m_reinfArea.fill(0,vReinf.size());
    m_reinfCenter.fill(QPointF(0,0),vReinf.size());
    for (int i=0; i<vReinf.size();++i)
    {
        m_reinfArea[i]=M_PI*qPow(vReinf[i].first,2)/4;
        m_reinfCenter[i]=vReinf[i].second;
        qDebug()<<"Center point of reinforcement bar:"+QString::number(i+1)+" is at x:"+QString::number(m_reinfCenter[i].x())+" y:"+QString::number(m_reinfCenter[i].y());
    }
}

void Calculation::setCenterPoint()
{
    double dStMomentY=0;
    double dStMomentX=0;
    for (int i=0; i<m_concreteArea.size(); ++i)
    {
        for (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            dStMomentX+=m_concreteArea[i][j]*m_concreteCenter[i][j].x();
            dStMomentY+=m_concreteArea[i][j]*m_concreteCenter[i][j].y();
            m_area+=m_concreteArea[i][j];
        }
    }
    m_centerPoint.setX(dStMomentX/m_area);
    m_centerPoint.setY(dStMomentY/m_area);
    qDebug()<<"Center Point x;"+QString::number(m_centerPoint.x())+" y:"+QString::number(m_centerPoint.y());
}

void Calculation::setMomentsOfInertia(const QVector<QVector<double>>& vJx, const QVector<QVector<double>>& vJy)
{
    for (int i=0; i<m_concreteArea.size(); ++i)
    {
        for (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            m_Jx+=vJx[i][j]+qPow(m_concreteCenter[i][j].y()-m_centerPoint.y(),2)*m_concreteArea[i][j];
            m_Jy+=vJy[i][j]+qPow(m_concreteCenter[i][j].x()-m_centerPoint.x(),2)*m_concreteArea[i][j];
        }
    }
    qDebug()<<"Jx="+QString::number(m_Jx)+" Jy="+QString::number(m_Jy);
}

void Calculation::setN(double d)
{
    m_N=d;
}

void Calculation::setMx(double d)
{
    m_Mx=d;
}

void Calculation::setMy(double d)
{
    m_My=d;
}

void Calculation::setAlfa()
{
    if(m_Mx==0||m_Jy==0)
    {
        if (m_My>0)
        {
            m_alfa=M_PI_2;
        }
        if (m_My<0)
        {
            m_alfa=-M_PI_2;
        }
        else
        {
            m_alfa=0;
        }
    }
    else
    {
        m_alfa=qAtan(m_Jx/m_Jy*m_My/m_Mx);
    }
}

void Calculation::slotSetEb(double d)
{
    m_Eb=d;
    qDebug()<<"Eb is set to "<< d;
}
