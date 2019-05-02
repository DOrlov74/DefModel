#ifndef CALCULATION_H
#define CALCULATION_H

#include <QObject>
#include <QVector>
#include <QPointF>

class Calculation : public QObject
{
    Q_OBJECT

    QVector<QVector<double>> m_concreteArea;
    QVector<QVector<QPointF>> m_concreteCenter;
    QVector<double> m_reinfArea;
    QVector<QPointF> m_reinfCenter;
    uint nXdivisions;    //Number of divisions for concrete section
    uint nYdivisions;
    QPointF m_centerPoint;
    double m_area=0;
    double m_Jy=0;
    double m_Jx=0;
    double m_N=0;
    double m_Mx=0;
    double m_My=0;

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

signals:

public slots:
};

#endif // CALCULATION_H
