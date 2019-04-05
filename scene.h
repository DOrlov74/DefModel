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
    //Points in GraphicsScene coordinats
    QVector<QPointF> m_concretePoints;
    //Path to draw concrete section
    QPainterPath* m_concretePath;
    QPen pen;
    QBrush brush;
    uint m_pointSize=5;
    QGraphicsEllipseItem* m_currentItem;
    //List to store points of concrete section
    QList<QGraphicsItem*> m_pointsItems;
    //QGraphicsItemGroup* m_pointsGroup;
    //Point to transform coordinats for display
    QPointF m_basePoint;
    //Actual size of concrete section
    double m_recWidth;
    double m_recHeight;
    //Number of divisions for concrete section
    uint nXdivisions=10;
    uint nYdivisions=10;

    //Transform coordinats methods
    QPointF toSceneCoord(const QPointF&);
    QPointF fromSceneCoord(const QPointF&);
    //Draw points of concrete section method
    void drawPoint(const QPointF&);


public:
    explicit Scene(QWidget *parent = nullptr);
    drawMode getDrawMode();
    void setBasePoint(QPointF);

signals:
    void signalDrawMode(bool);
    void signalPointAdded(QPointF);
    void signalSceneInit();

public slots:
    void setDrawLine();
    void setDrawRect();
    void slotGetCommand(QString);
    void slotSceneInit();
    void slotDivide();

    // QGraphicsScene interface
protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    virtual void timerEvent(QTimerEvent *event);
};

#endif // SCENE_H
