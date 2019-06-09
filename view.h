#ifndef VIEW_H
#define VIEW_H

#include <QWidget>
#include <QGraphicsView>
#include <QPointF>

class view : public QGraphicsView
{
    Q_OBJECT
    bool m_panMode=false;
    QPointF m_currPoint;

public:
    explicit view(QWidget *parent = nullptr);

signals:
    void signalViewInit();

public slots:
    void slotPan(bool);
    void slotZoomIn();
    void slotZoomOut();

    // QWidget interface
protected:
    virtual void wheelEvent(QWheelEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
};

#endif // VIEW_H
