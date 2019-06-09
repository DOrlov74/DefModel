#ifndef SCENE_H
#define SCENE_H

#include "calculation.h"
#include <QWidget>
#include <QGraphicsScene>
#include <QVector>
#include <QGraphicsItemGroup>
#include <QAbstractGraphicsShapeItem>
#include "excelinouthelper.h"
#include "infoform.h"

class Scene : public QGraphicsScene
{
    Q_OBJECT

    Calculation* myCalc;
    ExcelInOutHelper* myExcel;
    int idTimer;
    enum drawMode{NONE, LINE, RECT, POINT};
    drawMode m_drawMode=NONE;
    bool m_doneConcretePath=false;
    bool m_doneReinforcement=false;
    bool m_sectDivided=false;
    QVector<QPointF> m_concretePoints;  //Points of concrete section in GraphicsScene coordinats
    QVector<QVector<QPointF>> m_dividedPoints;   //Points of divided elements in GraphicsScene coordinats
    QVector<QVector<QVector<QPointF>>> m_dividedRegions;   //Points of divided regions in GraphicsScene coordinats
    QVector<QVector<QVector<QVector<int>>>> m_dividedFaces;          //Numbers of points for every face of divided rectangular
    QVector<QPair<uint,QPointF>> m_reinfCircles;    //container to store diameter and point of every reinforcement bar
    QPainterPath* m_concretePath;       //Path to draw concrete section
    QPainterPath* m_baseSignPath;       //Path to draw base sign
    uint m_currDiam=10;                    //current reinforcement diameter
    QPen pen=QPen(QColor(100,100,100));
    QBrush brush;
    QBrush bCompressed=QBrush(QColor(0,180,220),Qt::SolidPattern);           //Brush to draw compressed concrete
    QBrush bTensile=QBrush(QColor(200,240,255),Qt::SolidPattern);           //Brush to draw tensile concrete
    uint m_pointSize=5;
    QGraphicsEllipseItem* m_currentItem;
    QList<QGraphicsItem*> m_pointsItems;        //List to store points of concrete section
    QGraphicsItem* m_pathItem;                  //Pointer to the concrete path item
    QVector<QGraphicsItem*> m_baseSignItem;                  //Pointer to the base sign item
    QVector<QVector<QGraphicsRectItem*>> m_divisionItems;    //Container to store divided rectangles of concrete section
    QVector<QVector<QGraphicsPathItem*>> m_divisionPaths;    //Container to store divided paths of concrete section
    QList<QGraphicsEllipseItem*> m_reinfItems;      //List to store reinforcement circles
    QVector<QVector<double>> m_concreteArea;        //Areas of the divided elements
    QVector<QVector<double>> m_concreteJx;          //Moments of inertia about X axis of the divided elements
    QVector<QVector<double>> m_concreteJy;          //Moments of inertia about Y axis of the divided elements
    QVector<QVector<QPointF>> m_concreteCenter;     //Point in center of the divided elements
    QVector<QVector<QGraphicsSimpleTextItem*>> m_concreteResultText;    //Container to store text result of concrete elements
    QVector<QGraphicsSimpleTextItem*> m_reinfResultText;      //Container to store text result of reinforcement bars
    QGraphicsSimpleTextItem* m_resultTitle;                     //Pointer to the result title text
    //QGraphicsItemGroup* m_pointsGroup;
    QPointF m_basePoint;                    //Point to transform coordinats for display
    double m_recWidth;      //Actual size of concrete section
    double m_recHeight;
    double lowX;            //boundaries of the section
    double highX;
    double lowY;
    double highY;
    uint nXdivisions=10;    //Number of divisions for concrete section
    uint nYdivisions=10;
    uint m_viewMargin=20;
    bool m_isRect;
    bool m_leftToRight;
    bool m_NIsSet=false;
    bool m_MxIsSet=false;
    bool m_MyIsSet=false;
    bool m_resultIsSaved=false;
    int m_resultMode=1;             //flag to store output type
    bool m_saveToExcel=false;       //flag to store option to save result to excel
    bool m_calcIsSuccessful=false;          //flag to store if the calculation is successful
    uint m_fontSize;                        //store font size for result output
    QFont m_titleFont;                        //font style for title of result output
    QFont m_textFont;                         //font style for text of result output
    QBrush m_renfTextColor=QBrush(QColor(200,0,150));                   // color for reinforcement text result

    QPointF toSceneCoord(const QPointF&);   //Transform coordinats methods
    QPointF fromSceneCoord(const QPointF&);
    void drawPoint(const QPointF&);         //Draw points of concrete section method
    void getSectSizes();
    void setSceneSize();
    void drawDivisions();
    bool isBetween(QPointF, QPointF, QPointF);
    void Divide();
    void checkForces();
    void switchPoint(QPointF&, QPointF&);
    void DrawStress();
    void DrawStrain();
    void DrawArea();
    void clearResult();
    QGraphicsSimpleTextItem* fitFont(QGraphicsSimpleTextItem*);
    void setFontSize();

public:
    explicit Scene(QWidget *parent = nullptr);
    ~Scene();
    drawMode getDrawMode();
    void setBasePoint(QPointF);
    int getCurrDiam();
    void loadData();

signals:
    void signalDrawMode(bool);
    void signalPointAdded(QPointF);
    void signalSceneInit();
    void signalCoordChanged(QPointF);
    void signalSceneCleared(bool);
    void signalSceneDivided(bool);
    void signalSectDone(bool);
    void signalReinfDone(bool);
    void signalReinfCleared(bool);
    void signalFitView();
    void signalForcesDone(bool);
    //void signalGetRDiameter(int);
    void signalSetEb(double);
    void signalSetEs(double);
    void signalSetRb(double);
    void signalSetRbt(double);
    void signalSetRs(double);
    void signalCalcStart();
    void signalPercentChanged(int);
    void signalCalcEnd(bool);
    void signalExportStart();
    void signalExportPercentChanged(int);
    void signalExportEnd();

public slots:
    void setDrawLine();
    void setDrawRect();
    void setDrawPoint();
    void slotGetCommand(QString);
    void slotSceneInit();
    void slotNewSection();
    void slotDivideX(uint);
    void slotDivideY(uint);
    void slotSetRDiameter(int);
    void slotNewReinf();
    void slotCalculate();
    void slotFitView();
    void slotSetN(QString);
    void slotSetMx(QString);
    void slotSetMy(QString);
    void slotSetEb(double);
    void slotSetEs(double);
    void slotSetRb(double);
    void slotSetRbt(double);
    void slotSetRs(double);
    void slotImportPoints();
    void slotExportPoints();
    void slotCalcStart();
    void slotPercentChanged(int);
    void slotCalcEnd(bool);
    void slotDrawStress();
    void slotExportStart();
    void slotExportPercentChanged(int);
    void slotExportEnd();
    void slotApplyPressed(int);
    void slotSaveToExcel(bool);

    // QGraphicsScene interface
protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
//    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    virtual void timerEvent(QTimerEvent *event);
};

#endif // SCENE_H
