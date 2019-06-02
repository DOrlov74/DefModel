#include "calculation.h"
#include <QDebug>
#include <QPointF>
#include <QVector>
#include <QtMath>
#include <QProgressDialog>
#include <QMessageBox>

void Calculation::setMaxCStrain()
{
    for (int i=0; i<m_concreteStrain.size(); ++i)
    {
        for (int j=0; j<m_concreteStrain[i].size(); ++j)
        {
            if ((i==0)||(i==m_concreteStrain.size()-1)||(j==0)||(j==m_concreteStrain[i].size()-1))
            {
                if (m_concreteStrain[i][j]>m_MaxCStrain)
                {m_MaxCStrain=m_concreteStrain[i][j];}
                if (m_concreteStrain[i][j]<m_MinCStrain)
                {m_MinCStrain=m_concreteStrain[i][j];}
            }
        }
    }
}

void Calculation::setMaxRStrain()
{
    for (int i=0; i<m_reinfStrain.size(); ++i)
    {
        if (qAbs(m_reinfStrain[i])>m_MaxRStrain)
        {m_MaxRStrain=qAbs(m_reinfStrain[i]);}
    }
}

bool Calculation::checkStrain()
{
    QString resultString;
    if (m_MaxCStrain>m_ebt2)
    {
        resultString="Strain in concrete section "+ QString::number(m_MaxCStrain)+ " is bigger than the ultimate tensile strain "+QString::number(m_ebt2)+ ". Try to increase concrete or reinforcement section";
    }
    if (m_MinCStrain<(-m_eb2))
    {
        resultString="Strain in concrete section "+ QString::number(m_MaxCStrain)+ " is bigger than the ultimate compressive strain "+QString::number(-m_eb2)+ ". Try to increase concrete or reinforcement section";
    }
    if (m_MaxRStrain>m_es2)
    {
        resultString="Strain in reinforcement section "+ QString::number(m_MaxCStrain)+ " is bigger than the ultimate compressive strain "+QString::number(m_es2)+ ". Try to increase concrete or reinforcement section";
    }
    else
    {
        resultString="Calculation is finished successfiully";
        m_calcIsSuccessful=true;
        return true;
    }
    QMessageBox::information(nullptr,"Calculation result",resultString);
    return false;
}

const QVector<QVector<double> > &Calculation::getCStress()
{
    return m_concreteStress;
}

const QVector<double> &Calculation::getRStress()
{
    return m_reinfStress;
}

const QVector<QVector<double> > &Calculation::getCStrain()
{
    return m_concreteStrain;
}

const QVector<double> &Calculation::getRStrain()
{
    return m_reinfStrain;
}

const QVector<QVector<double> > &Calculation::getCArea()
{
    return m_concreteArea;
}

const QVector<double> &Calculation::getRArea()
{
    return m_reinfArea;
}

bool Calculation::saveResult()
{
    emit signalExportStart();
    emit signalExportPercentChanged(10);
    ExcelInOutHelper* myExcel=new ExcelInOutHelper(this);
    myExcel->saveArea(m_concreteArea, m_reinfArea);
    emit signalExportPercentChanged(30);
    myExcel->saveCenterDist(m_concreteCenter, m_reinfCenter);
    emit signalExportPercentChanged(40);
    myExcel->saveArea(m_concreteArea, m_reinfArea);
    emit signalExportPercentChanged(50);
    myExcel->saveCenterDist(m_concreteCenter, m_reinfCenter);
    emit signalExportPercentChanged(60);
    myExcel->saveKElast(m_KbElast, m_KrElast);
    emit signalExportPercentChanged(70);
    myExcel->savevEb(m_vEb);
    emit signalExportPercentChanged(80);
    myExcel->saveStrain(m_concreteStrain, m_reinfStrain);
    emit signalExportPercentChanged(90);
    myExcel->saveStress(m_concreteStress, m_reinfStress);
    emit signalExportPercentChanged(100);
    emit signalExportEnd();
}

Calculation::Calculation(QObject *parent) : QObject(parent)
{
   //myInfo=new InfoForm();
}

Calculation::~Calculation()
{
    //delete myInfo;
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
            m_concreteArea[i][j]=vArea[i][j]/1000000;
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
            m_concreteCenter[i][j].setX(vCenter[i][j].x()/1000);
            m_concreteCenter[i][j].setY(vCenter[i][j].y()/1000);
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
        m_reinfArea[i]=M_PI*qPow(vReinf[i].first,2)/4000000;
        m_reinfCenter[i].setX(vReinf[i].second.x()/1000);
        m_reinfCenter[i].setY(vReinf[i].second.y()/1000);
        qDebug()<<"Center point of reinforcement bar:"+QString::number(i+1)+" is at x:"+QString::number(m_reinfCenter[i].x())+" y:"+QString::number(m_reinfCenter[i].y());
    }
}

void Calculation::setCenterPoint()
{
    double dStMomentY=0;
    double dStMomentX=0;
    m_area=0;
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
    m_Jx=0;
    m_Jy=0;
    for (int i=0; i<m_concreteArea.size(); ++i)
    {
        for (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            m_Jx+=vJx[i][j]/1000000000000+qPow(m_concreteCenter[i][j].y()-m_centerPoint.y(),2)*m_concreteArea[i][j];
            m_Jy+=vJy[i][j]/1000000000000+qPow(m_concreteCenter[i][j].x()-m_centerPoint.x(),2)*m_concreteArea[i][j];
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
    m_Mx=-d;
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
    if ((d>=m_ebt2)||(d<=-m_eb2))
    {
        return 0;
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
    else if(d<=-m_es2||d>=m_es2)
    {
        return 0;
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

void Calculation::setStartvEb()
{
    m_vEb.fill(QVector<double>(),nXdivisions);
    for (int i=0; i<nXdivisions;++i)
    {
        m_vEb[i].fill(m_Eb_red,nYdivisions);
    }
}

void Calculation::setD11()
{
    m_D11=0;
    for (int i=0; i<m_concreteArea.size(); ++i)
    {
        for (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            m_D11+=m_concreteArea[i][j]*qPow(m_concreteCenter[i][j].x(),2)*m_Eb_red*m_KbElast[i][j];
        }
    }
    for (int i=0; i<m_reinfArea.size(); ++i)
    {
        m_D11+=m_reinfArea[i]*qPow(m_reinfCenter[i].x(),2)*m_Es*m_KrElast[i];
    }
}

void Calculation::setD22()
{
    m_D22=0;
    for (int i=0; i<m_concreteArea.size(); ++i)
    {
        for (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            m_D22+=m_concreteArea[i][j]*qPow(m_concreteCenter[i][j].y(),2)*m_Eb_red*m_KbElast[i][j];
        }
    }
    for (int i=0; i<m_reinfArea.size(); ++i)
    {
        m_D22+=m_reinfArea[i]*qPow(m_reinfCenter[i].y(),2)*m_Es*m_KrElast[i];
    }
}

void Calculation::setD12()
{
    m_D12=0;
    for (int i=0; i<m_concreteArea.size(); ++i)
    {
        for (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            m_D12+=m_concreteArea[i][j]*m_concreteCenter[i][j].x()*m_concreteCenter[i][j].y()*m_Eb_red*m_KbElast[i][j];
        }
    }
    for (int i=0; i<m_reinfArea.size(); ++i)
    {
        m_D12+=m_reinfArea[i]*m_reinfCenter[i].x()*m_reinfCenter[i].y()*m_Es*m_KrElast[i];
    }
}

void Calculation::setD13()
{
    m_D13=0;
    for (int i=0; i<m_concreteArea.size(); ++i)
    {
        for (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            m_D13+=m_concreteArea[i][j]*m_concreteCenter[i][j].x()*m_Eb_red*m_KbElast[i][j];
        }
    }
    for (int i=0; i<m_reinfArea.size(); ++i)
    {
        m_D13+=m_reinfArea[i]*m_reinfCenter[i].x()*m_Es*m_KrElast[i];
    }
}

void Calculation::setD23()
{
    m_D23=0;
    for (int i=0; i<m_concreteArea.size(); ++i)
    {
        for (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            m_D23+=m_concreteArea[i][j]*m_concreteCenter[i][j].y()*m_Eb_red*m_KbElast[i][j];
        }
    }
    for (int i=0; i<m_reinfArea.size(); ++i)
    {
        m_D23+=m_reinfArea[i]*m_reinfCenter[i].y()*m_Es*m_KrElast[i];
    }
}

void Calculation::setD33()
{
    m_D33=0;
    for (int i=0; i<m_concreteArea.size(); ++i)
    {
        for (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            m_D33+=m_concreteArea[i][j]*m_Eb_red*m_KbElast[i][j];
        }
    }
    for (int i=0; i<m_reinfArea.size(); ++i)
    {
        m_D33+=m_reinfArea[i]*m_Es*m_KrElast[i];
    }
}

double Calculation::findCurv()
{
    double prev_1rx=m_1rx;
    double prev_1ry=m_1ry;
    double prev_e0=m_e0;
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
//        qDebug()<<"1rx="<<m_1rx<<"1ry="<<m_1ry<<"e0="<<m_e0;
    }
    else if (m_D12!=0)
    {
        m_1ry=(m_Mx-m_D11*m_1rx-m_D13*m_e0)/m_D12;
        m_1rx=(m_My-m_D22*m_1ry-m_D23*m_e0)/m_D12;
        if (m_D33!=0)
        {
            m_e0=(m_N-m_D13*m_1rx-m_D23*m_1ry)/m_D33;
        }
//        qDebug()<<"1rx="<<m_1rx<<"1ry="<<m_1ry<<"e0="<<m_e0;
    }
    else if (m_D13!=0)
    {
        m_e0=(m_Mx-m_D11*m_1rx-m_D12*m_1ry)/m_D13;
        m_1rx=(m_N-m_D23*m_1ry-m_D33*m_e0)/m_D13;
        if(m_D22!=0)
        {
            m_1ry=(m_My-m_D12*m_1rx-m_D23*m_e0)/m_D22;
        }
 //       qDebug()<<"1rx="<<m_1rx<<"1ry="<<m_1ry<<"e0="<<m_e0;
    }
    else
    {
        qDebug()<<"can't find curvature solution";
    }
    return max(qFabs(m_1rx-prev_1rx),qFabs(m_1ry-prev_1ry),qFabs(m_e0-prev_e0));
}

void Calculation::setStrain()
{
    m_concreteStrain.fill(QVector<double>(),nXdivisions);
    for (int i=0; i<m_concreteArea.size();++i)
    {
        m_concreteStrain[i].fill(0,nYdivisions);
        for  (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            m_concreteStrain[i][j]=m_e0+m_1rx*m_concreteCenter[i][j].x()+m_1ry*m_concreteCenter[i][j].y();
//           qDebug()<<"Strain of concrete part in row:"+QString::number(i+1)+" in column:"+QString::number(j+1)+" is:"+QString::number(m_concreteStrain[i][j]);
        }
    }
    m_reinfStrain.fill(0,m_reinfArea.size());
    for (int i=0; i<m_reinfArea.size();++i)
    {
        m_reinfStrain[i]=m_e0+m_1rx*m_reinfCenter[i].x()+m_1ry*m_reinfCenter[i].y();
//        qDebug()<<"Strain of reinforcement element:"+QString::number(m_reinfStrain[i]);
    }
}

void Calculation::setStress()
{
    m_concreteStress.fill(QVector<double>(),nXdivisions);
    for (int i=0; i<m_concreteArea.size();++i)
    {
        m_concreteStress[i].fill(0,nYdivisions);
        for  (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            m_concreteStress[i][j]=SigmaB(m_concreteStrain[i][j]);  //*m_KbElast[i][j]
//            qDebug()<<"Stress in concrete part in row:"+QString::number(i+1)+" in column:"+QString::number(j+1)+" is:"+QString::number(m_concreteStress[i][j]);
        }
    }
    m_reinfStress.fill(0,m_reinfArea.size());
    for (int i=0; i<m_reinfArea.size();++i)
    {
        m_reinfStress[i]=SigmaS(m_reinfStrain[i]);  //*m_KrElast[i]
//        qDebug()<<"Stress in reinforcement element:"+QString::number(m_reinfStress[i]);
    }
}

double Calculation::checkForces()
{
    double calc_Mx=0;
    double calc_My=0;
    double calc_N=0;
    for (int i=0; i<m_concreteArea.size();++i)
    {
        for  (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            calc_Mx+=m_concreteStress[i][j]*m_concreteArea[i][j]*m_concreteCenter[i][j].x();
            calc_My+=m_concreteStress[i][j]*m_concreteArea[i][j]*m_concreteCenter[i][j].y();
            calc_N+=m_concreteStress[i][j]*m_concreteArea[i][j];
        }
    }
    for (int i=0; i<m_reinfArea.size();++i)
    {
        calc_Mx+=m_reinfStress[i]*m_reinfArea[i]*m_reinfCenter[i].x();
        calc_My+=m_reinfStress[i]*m_reinfArea[i]*m_reinfCenter[i].y();
        calc_N+=m_reinfStress[i]*m_reinfArea[i];
    }
    return max(qFabs(m_Mx-calc_Mx),qFabs(m_My-calc_My),qFabs(m_N-calc_N));
}

void Calculation::setKElast()
{
    for (int i=0; i<m_concreteArea.size();++i)
    {
        for  (int j=0; j<m_concreteArea[i].size(); ++j)
        {
            if (m_concreteStrain[i][j]!=0)
            {
                m_KbElast[i][j]=m_concreteStress[i][j]/(m_concreteStrain[i][j]*(m_concreteStrain[i][j]>0?m_Ebt_red:m_Eb_red));
                m_vEb[i][j]=(m_concreteStrain[i][j]>0?m_Ebt_red:m_Eb_red);
            }
        }
    }
    for (int i=0; i<m_reinfArea.size();++i)
    {
        if (m_reinfStrain[i]!=0)
        {
            m_KrElast[i]=m_reinfStress[i]/(m_reinfStrain[i]*m_Es);
        }
    }
}

double Calculation::max(double d1, double d2, double d3)
{
    if(d1>d2&&d1>d3)
    {return d1;}
    else if (d2>d1&&d2>d3)
    {return d2;}
    else
    {return d3;}
}

void Calculation::calculate()
{
    //QProgressDialog* myInfo= new QProgressDialog("Calculation in progress...","Cancel",0,100);    //Не работает
    //myInfo->setWindowModality(Qt::NonModal);
    //myInfo->resize(300,200);
    //myInfo->setWindowTitle("Warning");
    //waitDialog->addButton(QMessageBox::Cancel);
    //waitDialog->setText("Wait please...");
    //myInfo->show();
    emit signalCalcStart();
    double curAccuracy=0;
    int cur_it;
    emit signalPercentChanged(10);
    setStartKbElast();
    setStartKrElast();
    setStartvEb();
    emit signalPercentChanged(20);
    for (cur_it=1; cur_it<=nIterations; ++cur_it)
    {
        double innerAccuracy=0;
        setD11();
        setD22();
        setD12();
        setD13();
        setD23();
        setD33();
        for (int innerIt=1; innerIt<=nIterations; ++innerIt)
        {
            innerAccuracy=findCurv();
            qDebug()<<"iteration:"<<innerIt<<" accuracy in curvature:"<<innerAccuracy;
            if (innerAccuracy*1000<m_accuracy)
            {break;}
        }
        setStrain();
        setStress();
        setKElast();
        curAccuracy=checkForces();
        qDebug()<<"iteration:"<<cur_it<<" accuracy in equilibrium equation:"<<curAccuracy;
        if (curAccuracy*100<m_accuracy)
        {break;}
    }
    emit signalPercentChanged(80);
    if (cur_it>nIterations)
    {
        if (curAccuracy>m_accuracy)
        {
            QString resultString="Could not reach target accurancy. Try to increase concrete or reinforcement section";
            QMessageBox::information(nullptr,"Calculation result",resultString);
            return;
        }
    }
      setMaxCStrain();
      setMaxRStrain();
      checkStrain();
      if (m_calcIsSuccessful)
      {
          emit signalDrawStress();
      }
      emit signalPercentChanged(100);
      emit signalCalcEnd(m_calcIsSuccessful);   //cancel();
}

void Calculation::slotSetEb(double d)
{
    m_Eb=d*1000;
    qDebug()<<"Eb is set to "<< d;
}

void Calculation::slotSetEs(double d)
{
    m_Es=d*1000;
    qDebug()<<"Es is set to "<< d;
}

void Calculation::slotSetRb(double d)
{
    m_Rb=d*1000;
    qDebug()<<"Rb is set to "<< d;
    m_Eb_red=m_Rb/m_eb1_red;
}

void Calculation::slotSetRbt(double d)
{
    m_Rbt=d*1000;
    qDebug()<<"Rbt is set to "<< d;
    m_Ebt_red=m_Rbt/m_ebt1_red;
}

void Calculation::slotSetRs(double d)
{
    m_Rs=d*1000;
    qDebug()<<"Rs is set to "<< d;
    m_es0=m_Rs/m_Es;
}
