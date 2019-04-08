#ifndef SCENE_H
#define SCENE_H

#include <QWidget>
#include <QGraphicsScene>
#include <QVector>
#include <QGraphicsItemGroup>
#include <QAbstractGraphicsShapeItem>

class Scene : public QGraphicsScene
{
    Q_OBJECT

    int idTimer;
    enum drawMode{NONE, LINE, RECT, CIRCLE};
    drawMode m_drawMode=NONE;
    bool m_doneConcretePath=false;
    QVector<QPointF> m_concretePoints;  //Points in GraphicsScene coordinats
    QPainterPath* m_concretePath;       //Path to draw concrete section
    QPen pen;
    QBrush brush;
    uint m_pointSize=5;
    QGraphicsEllipseItem* m_currentItem;
    QList<QGraphicsItem*> m_pointsItems;    //List to store points of concrete section
    //QGraphicsItemGroup* m_pointsGroup;
    QPointF m_basePoint;                    //Point to transform coordinats for display
    double m_recWidth;      //Actual size of concrete section
    double m_recHeight;
    uint nXdivisions=10;    //Number of divisions for concrete section
    uint nYdivisions=10;
    uint m_viewMargin=20;
    bool m_isRect;
    bool m_leftToRight;

    QPointF toSceneCoord(const QPointF&);   //Transform coordinats methods
    QPointF fromSceneCoord(const QPointF&);
    void drawPoint(const QPointF&);         //Draw points of concrete section method
    void getSectSizes();

public:
    explicit Scene(QWidget *parent = nullptr);
    drawMode getDrawMode();
    void setBasePoint(QPointF);

signals:
    void signalDrawMode(bool);
    void signalPointAdded(QPointF);
    void signalSceneInit();
    void signalCoordChanged(QPointF);
    void signalSceneCleared(bool);

public slots:
    void setDrawLine();
    void setDrawRect();
    void slotGetCommand(QString);
    void slotSceneInit();
    void slotDivide();
    void slotNewSection();

    // QGraphicsScene interface
protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
//    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    virtual void timerEvent(QTimerEvent *event);
};

#endif // SCENE_H
