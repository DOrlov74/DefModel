#ifndef CALCULATION_H
#define CALCULATION_H

#include <QObject>
#include <QVector>
#include <QPointF>
#include "excelinouthelper.h"

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
    double m_Eb=0;              //Modulus of elasticity of concrete
    double m_Eb_red=0;          //Given modulus of elasticity of compressed concrete
    double m_Es=0;              //Modulus of elasticity of reinforcement steel
    double m_Rb=0;              //Compressive strength of concrete
    double m_Rbt=0;             //Tensile strength of concrete
    double m_Ebt_red=0;         //Given modulus of elasticity of tensile concrete
    double m_Rs=0;              //Tensile strength of reinforcement steel
    const double m_eb1_red=0.0015;      //Strain at reaching the maximum concrete compressive strength
    const double m_eb2=0.0035;          //The ultimate concrete compressive strain
    const double m_ebt1_red=0.0008;     //Strain at reaching the maximum concrete tensile strength
    const double m_ebt2=0.00015;        //The ultimate concrete tensile strain
    double m_es0=0;                     //Strain at reaching the maximum reinforcement steel tensile strength
    const double m_es2=0.025;           //The ultimate reinforcement steel tensile strain
    QVector<QVector<double>> m_KbElast; //Coefficients of elasticity of concrete elements
    QVector<double> m_KrElast;          //Coefficients of elasticity of reinforcement elements
    //stiffness characteristics
    double m_D11=0;
    double m_D22=0;
    double m_D12=0;
    double m_D13=0;
    double m_D23=0;
    double m_D33=0;
    double m_1rx=0;             //Curvature of the element in X axis plane
    double m_1ry=0;             //Curvature of the element in Y axis plane
    double m_e0=0;              //strain of the center point of the element
    double m_accuracy=0.001;    //accuracy of solution
    int nIterations=10;            //number of iterations
    QVector<QVector<double>> m_concreteStrain;        //Strain of concrete elements
    QVector<double> m_reinfStrain;                    //Strain of reinforcement elements
    QVector<QVector<double>> m_concreteStress;        //Stress in concrete elements
    QVector<double> m_reinfStress;                    //Stress in reinforcement elements

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
    double SigmaB(double);          //Method to calculate stress from strain in concrete section
    double SigmaS(double);          //Method to calculate stress from strain in reinforcement section
    void setStartKbElast();         //Fill container with coefficients of elasticity of concrete elements with 1s
    void setStartKrElast();         //Fill container with coefficients of elasticity of reinforcement elements with 1s
    //Methods to calculate stiffness characteristics
    void setD11();
    void setD22();
    void setD12();
    void setD13();
    void setD23();
    void setD33();
    double findCurv();              //find curvature and strain of center point and return current accuracy
    void setStrain();               //calculate strain of concrete and reinforcement elements
    void setStress();               //calculate stress in concrete and reinforcement elements
    double checkForces();               //Check equilibrium equations and return current accuracy
    void setKElast();                   //Calculate coefficients of elasticity of concrete and reinforcement elements based on current strain and stress
    double max(double,double,double);   //find maximum of three double numbers
    void calculate();                   //The main method to calculate section

signals:

public slots:
    void slotSetEb(double);
    void slotSetEs(double);
    void slotSetRb(double);
    void slotSetRs(double);

};

#endif // CALCULATION_H
