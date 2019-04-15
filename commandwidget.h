#ifndef COMMANDWIDGET_H
#define COMMANDWIDGET_H

#include <QWidget>
#include <QTextEdit>

class commandWidget : public QTextEdit
{
    Q_OBJECT
    uint n_points=0;
    uint m_strPos=0;
    QString m_strCommand;
    bool m_DivideMode=false;
    bool m_drawPointMode=false;
    bool m_waitPoint=false;
    bool m_waitX=false;
    bool m_waitY=false;
public:
    explicit commandWidget(QWidget *parent = nullptr);

signals:
    void signalCommand(QString);
    void signalDivideX(uint);
    void signalDivideY(uint);

public slots:
    void slotDrawLine();
    void slotDrawRect();
    void slotDrawPoint();
    void slotAddPoint(QPointF);
    void slotAddKey(QString);
    void slotGetText();
    void slotDivide();
    void slotDrawMode(bool);

    // QWidget interface
protected:
//    virtual void keyPressEvent(QKeyEvent *event);
};

#endif // COMMANDWIDGET_H
