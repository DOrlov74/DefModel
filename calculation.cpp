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
