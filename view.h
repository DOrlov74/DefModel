#ifndef VIEW_H
#define VIEW_H

#include <QWidget>
#include <QGraphicsView>

class view : public QGraphicsView
{
    Q_OBJECT
public:
    explicit view(QWidget *parent = nullptr);

signals:
    void signalViewInit();

public slots:
};

#endif // VIEW_H
