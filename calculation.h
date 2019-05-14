#ifndef CALCULATION_H
#define CALCULATION_H

#include <QObject>
#include <QVector>
#include <QPointF>

class Calculation : public QObject
{
    Q_OBJECT

    QVector<QVector<double>> m_concreteArea;        //Areas of concrete elements
    QVector<QVector<QPointF>> m_concreteCenter;     //Coordinats of concrete element centers
    QVector<double> m_reinfArea;                    //Area of reinforcement elements
    QVector<QPointF> m_reinfCenter;                 //Coordinats of reinforcement centers
    uint nXdivisions;    //Number of divisions for concrete section
    uint nYdivisions;
    QPointF m_centerPoint;      //Center point coordinate of full concrete section
    double m_area=0;            //Area of full concrete section
    double m_Jy=0;              //Moment of inertia of full concrete section about Y axis
    double m_Jx=0;              //Moment of inertia of full concrete section about X axis
    double m_N=0;               //External normal stretching force
    double m_Mx=0;              //External moment along X axis
    double m_My=0;              //External moment along Y axis
    double m_alfa=0;            //Angle of neutral axis along resultant moment direction
    double m_Eb=0;              //Elastic modulus of concrete
    double m_Es=0;              //Elastic modulus of reinforcement steel

public:
    explicit Calculation(QObject *parent = nullptr);
    void setXdivision(uint);
    void setYdivision(uint);
    void setConcreteArea(const QVector<QVector<double>>&);
    void setConcreteCenter(const QVector<QVector<QPointF>>&);
    void setReinfArea(const QVector<QPair<uint,QPointF>>&);
    void setCenterPoint();
    void setMomentsOfInertia(const QVector<QVector<double>>&, const QVector<QVector<double>>&);
    void setN(double);
    void setMx(double);
    void setMy(double);
    void setAlfa();

signals:

public slots:
    void slotSetEb(double);
};

#endif // CALCULATION_H
