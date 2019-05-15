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
    for (int i=0; i<m_concreteCenter.size(); ++i)
    {
        for (int j=0; j<m_concreteCenter[i].size(); ++j)
        {
            m_concreteCenter[i][j].setX(m_concreteCenter[i][j].x()-m_centerPoint.x());
            m_concreteCenter[i][j].setY(m_concreteCenter[i][j].y()-m_centerPoint.y());
        }
    }
    for (int i=0; i<m_reinfCenter.size(); ++i)
    {
        m_reinfCenter[i].setX(m_reinfCenter[i].x()-m_centerPoint.x());
        m_reinfCenter[i].setY(m_reinfCenter[i].y()-m_centerPoint.y());
    }
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

double Calculation::SigmaB(double d)
{
    if (d>=m_ebt1_red&&d<m_ebt2)
    {
        return m_Rbt;
    }
    else if(d<=-m_eb1_red&&d>-m_eb2)
    {
        return -m_Rb;
    }
    else if(d<0)
    {
        return d*m_Eb_red;
    }
    else
    {
        return d*m_Ebt_red;
    }
}

double Calculation::SigmaS(double d)
{
    if (d>=m_es0&&d<m_es2)
    {
        return m_Rs;
    }
    else if(d<=-m_es0&&d>-m_es2)
    {
        return -m_Rs;
    }
    else
    {
        return d*m_Es;
    }
}

void Calculation::setStartKbElast()
{
    m_KbElast.fill(QVector<double>(),nXdivisions);
    for (int i=0; i<nXdivisions;++i)
    {
        m_KbElast[i].fill(1,nYdivisions);
    }
}

void Calculation::setStartKrElast()
{
    m_KrElast.fill(1,m_reinfArea.size());
}

void Calculation::setD11()
{
    for (int i=0; i<m_concreteArea.size(); ++i)
    {
        for (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            m_D11+=m_concreteArea[i][j]*qPow(m_concreteCenter[i][j].x(),2)*m_Eb*m_KbElast[i][j];
        }
    }
    for (int i=0; i<m_reinfArea.size(); ++i)
    {
        m_D11+=m_reinfArea[i]*qPow(m_reinfCenter[i].x(),2)*m_Es*m_KrElast[i];
    }
}

void Calculation::setD22()
{
    for (int i=0; i<m_concreteArea.size(); ++i)
    {
        for (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            m_D22+=m_concreteArea[i][j]*qPow(m_concreteCenter[i][j].y(),2)*m_Eb*m_KbElast[i][j];
        }
    }
    for (int i=0; i<m_reinfArea.size(); ++i)
    {
        m_D22+=m_reinfArea[i]*qPow(m_reinfCenter[i].y(),2)*m_Es*m_KrElast[i];
    }
}

void Calculation::setD12()
{
    for (int i=0; i<m_concreteArea.size(); ++i)
    {
        for (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            m_D12+=m_concreteArea[i][j]*m_concreteCenter[i][j].x()*m_concreteCenter[i][j].y()*m_Eb*m_KbElast[i][j];
        }
    }
    for (int i=0; i<m_reinfArea.size(); ++i)
    {
        m_D12+=m_reinfArea[i]*m_reinfCenter[i].x()*m_reinfCenter[i].y()*m_Es*m_KrElast[i];
    }
}

void Calculation::setD13()
{
    for (int i=0; i<m_concreteArea.size(); ++i)
    {
        for (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            m_D13+=m_concreteArea[i][j]*m_concreteCenter[i][j].x()*m_Eb*m_KbElast[i][j];
        }
    }
    for (int i=0; i<m_reinfArea.size(); ++i)
    {
        m_D13+=m_reinfArea[i]*m_reinfCenter[i].x()*m_Es*m_KrElast[i];
    }
}

void Calculation::setD23()
{
    for (int i=0; i<m_concreteArea.size(); ++i)
    {
        for (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            m_D23+=m_concreteArea[i][j]*m_concreteCenter[i][j].y()*m_Eb*m_KbElast[i][j];
        }
    }
    for (int i=0; i<m_reinfArea.size(); ++i)
    {
        m_D23+=m_reinfArea[i]*m_reinfCenter[i].y()*m_Es*m_KrElast[i];
    }
}

void Calculation::setD33()
{
    for (int i=0; i<m_concreteArea.size(); ++i)
    {
        for (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            m_D33+=m_concreteArea[i][j]*m_Eb*m_KbElast[i][j];
        }
    }
    for (int i=0; i<m_reinfArea.size(); ++i)
    {
        m_D33+=m_reinfArea[i]*m_Es*m_KrElast[i];
    }
}

void Calculation::findCurv()
{
    if (m_D11!=0)
    {
        m_1rx=(m_Mx-m_D12*m_1ry-m_D13*m_e0)/m_D11;
        if (m_D22!=0)
        {
            m_1ry=(m_My-m_D12*m_1rx-m_D23*m_e0)/m_D22;
            if (m_D33!=0)
            {
                m_e0=(m_N-m_D13*m_1rx-m_D23*m_1ry)/m_D33;
            }
        }
        qDebug()<<"1rx="<<m_1rx<<"1ry="<<m_1ry<<"e0="<<m_e0;
    }
    else if (m_D12!=0)
    {
        m_1ry=(m_Mx-m_D11*m_1rx-m_D13*m_e0)/m_D12;
        m_1rx=(m_My-m_D22*m_1ry-m_D23*m_e0)/m_D12;
        if (m_D33!=0)
        {
            m_e0=(m_N-m_D13*m_1rx-m_D23*m_1ry)/m_D33;
        }
        qDebug()<<"1rx="<<m_1rx<<"1ry="<<m_1ry<<"e0="<<m_e0;
    }
    else if (m_D13!=0)
    {
        m_e0=(m_Mx-m_D11*m_1rx-m_D12*m_1ry)/m_D13;
        m_1rx=(m_N-m_D23*m_1ry-m_D33*m_e0)/m_D13;
        if(m_D22!=0)
        {
            m_1ry=(m_My-m_D12*m_1rx-m_D23*m_e0)/m_D22;
        }
        qDebug()<<"1rx="<<m_1rx<<"1ry="<<m_1ry<<"e0="<<m_e0;
    }
    else
    {
        qDebug()<<"can't find solution";
    }
}

void Calculation::slotSetEb(double d)
{
    m_Eb=d;
    qDebug()<<"Eb is set to "<< d;
}

void Calculation::slotSetEs(double d)
{
    m_Es=d;
    qDebug()<<"Es is set to "<< d;
}

void Calculation::slotSetRb(double d)
{
    m_Rb=d;
    qDebug()<<"Rb is set to "<< d;
    m_Eb_red=m_Rb/m_eb1_red;
    m_Ebt_red=m_Rbt/m_ebt1_red;
}

void Calculation::slotSetRs(double d)
{
    m_Rs=d;
    qDebug()<<"Rs is set to "<< d;
}
