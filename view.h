#ifndef VIEW_H
#define VIEW_H

#include <QWidget>
#include <QGraphicsView>

class view : public QGraphicsView
{
    Q_OBJECT
    bool m_panMode=false;

public:
    explicit view(QWidget *parent = nullptr);

signals:
    void signalViewInit();

public slots:
    void slotPan(bool);

    // QWidget interface
protected:
    virtual void wheelEvent(QWheelEvent *event);
};

#endif // VIEW_H
