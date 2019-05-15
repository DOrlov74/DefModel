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
    QMap<QString,QPair<double,double>> m_cClasses;        //container to store concrete classes with modulus of elasticity and tensile strength
    QMap<QString,QPair<double,double>> m_rClasses;        //container to store reinforcement classes with modulus of elasticity and tensile strength

    void fillCClasses();
    void fillCCombobox();
    void fillRClasses();
    void fillRCombobox();

signals:
    void signalKeyPressed(QString);
    void signalSetEb(double);
    void signalSetEs(double);
    void signalSetRb(double);
    void signalSetRs(double);

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
    void slotCClassChanged(QString);
    void slotRClassChanged(QString);

private:
    Ui::MainWindow *ui;

    // QWidget interface
protected:
    virtual void keyPressEvent(QKeyEvent *event);

};

#endif // MAINWINDOW_H
