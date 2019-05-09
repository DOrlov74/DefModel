#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "scene.h"
#include "view.h"
#include "excelinouthelper.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Scene* myScene;

signals:
    void signalKeyPressed(QString);

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void setSceneSize();
//    void drawLineToggled(bool);
//    void drawRectToggled(bool);
//    void divideToggled(bool);
//    void newSectionToggled(bool);
    void slotFitView();
    void slotZoomIn();
    void slotZoomOut();
    void slotCoordChanged(QPointF);

private:
    Ui::MainWindow *ui;

    // QWidget interface
protected:
    virtual void keyPressEvent(QKeyEvent *event);

};

#endif // MAINWINDOW_H
