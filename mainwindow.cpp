#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "scene.h"
#include <QKeyEvent>
#include <QKeySequence>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    myScene= new Scene();
    ui->graphicsView->setScene(myScene);
    myScene->setSceneRect(0,0,ui->graphicsView->width(),ui->graphicsView->height());
    //qDebug()<<"scene width: "<< ui->graphicsView->width()<<"scene height: "<<ui->graphicsView->height();
    //myScene->setBasePoint(QPointF(10,myScene->height()-10));
    ui->actionNewSection->setDisabled(true);
    ui->actiondivide->setDisabled(true);
    QCursor cursor;
    cursor.setShape(Qt::CrossCursor);
    ui->graphicsView->setCursor(cursor);
    QObject::connect(ui->drawLineButton, SIGNAL(clicked()),ui->actionLine, SLOT(trigger()));          //drawline event
    QObject::connect(ui->drawRectButton, SIGNAL(clicked()),ui->actionRectangle, SLOT(trigger()));          //drawRectangle event
    QObject::connect(ui->divideButton, SIGNAL(clicked()), ui->actiondivide, SLOT(trigger()));             //divide section event
    QObject::connect(ui->newButton, SIGNAL(clicked()), ui->actionNewSection, SLOT(trigger()));            // new section event
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->drawLineButton, SLOT(setEnabled(bool)));    //enable/disable drawLine Button
    QObject::connect(myScene, SIGNAL(signalSceneCleared(bool)), ui->drawLineButton, SLOT(setEnabled(bool)));
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->drawRectButton, SLOT(setEnabled(bool)));    //enable/disable drawRectangle Button
    QObject::connect(myScene, SIGNAL(signalSceneCleared(bool)), ui->drawRectButton, SLOT(setEnabled(bool)));
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->newButton, SLOT(setDisabled(bool)));        //enable/disable new section Button
    QObject::connect(myScene, SIGNAL(signalSceneCleared(bool)), ui->newButton, SLOT(setDisabled(bool)));
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->actionNewSection, SLOT(setDisabled(bool)));
    QObject::connect(myScene, SIGNAL(signalSceneCleared(bool)), ui->actionNewSection, SLOT(setDisabled(bool)));
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->divideButton, SLOT(setDisabled(bool)));     //enable/disable divide section Button
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->actiondivide, SLOT(setDisabled(bool)));
    QObject::connect(myScene, SIGNAL(signalSceneDivided(bool)), ui->divideButton, SLOT(setDisabled(bool)));     //disable divide section Button
    QObject::connect(myScene, SIGNAL(signalSceneDivided(bool)), ui->actiondivide, SLOT(setDisabled(bool)));
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->actionLine, SLOT(setEnabled(bool)));        //enable/disable drawLine action
    QObject::connect(myScene, SIGNAL(signalSceneCleared(bool)), ui->actionLine, SLOT(setEnabled(bool)));
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->actionRectangle, SLOT(setEnabled(bool)));   //enable/disable drawRectangle action
    QObject::connect(myScene, SIGNAL(signalSceneCleared(bool)), ui->actionRectangle, SLOT(setEnabled(bool)));
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->commandTextEdit, SLOT(slotDrawMode(bool))); //send a signal of changing mode to textEdit
    QObject::connect(myScene, SIGNAL(signalCoordChanged(QPointF)), this, SLOT(slotCoordChanged(QPointF)));  //get actual coordinats from Scene
    QObject::connect(ui->actionFit_to_section, SIGNAL(triggered()), this, SLOT(slotFitView()));             //try to fit view
    QObject::connect(ui->actionLine, SIGNAL(triggered()), myScene, SLOT(setDrawLine()));                    //send command drawLine to Scene
    QObject::connect(ui->actionLine, SIGNAL(triggered()), ui->commandTextEdit, SLOT(slotDrawLine()));       //send command drawLine to textEdit
    QObject::connect(ui->actionRectangle, SIGNAL(triggered()), myScene, SLOT(setDrawRect()));               //send command drawRectangle to Scene
    QObject::connect(ui->actionRectangle, SIGNAL(triggered()), ui->commandTextEdit, SLOT(slotDrawRect()));  //send command drawRectangle to textEdit
    QObject::connect(ui->actiondivide, SIGNAL(triggered()), ui->commandTextEdit, SLOT(slotDivide()));       //send command divide to textEdit
    QObject::connect(ui->commandTextEdit, SIGNAL(signalDivideX(uint)), myScene, SLOT(slotDivideX(uint)));   //send number of divisions on axis X to Scene
    QObject::connect(ui->commandTextEdit, SIGNAL(signalDivideY(uint)), myScene, SLOT(slotDivideY(uint)));   //send number of divisions on axis Y to Scene
    QObject::connect(ui->actionNewSection, SIGNAL(triggered()), myScene, SLOT(slotNewSection()));           //send command new section to Scene
    QObject::connect(myScene, SIGNAL(signalPointAdded(QPointF)), ui->commandTextEdit, SLOT(slotAddPoint(QPointF))); //send added point to textEdit
    QObject::connect(this, SIGNAL(signalKeyPressed(QString)), ui->commandTextEdit, SLOT(slotAddKey(QString)));      //send command key from mainWidget to textEdit
    QObject::connect(this, SIGNAL(signalKeyPressed(QString)), myScene, SLOT(slotGetCommand(QString)));              //send command key from mainWidget to Scene
    QObject::connect(ui->commandTextEdit, SIGNAL(signalCommand(QString)), myScene, SLOT(slotGetCommand(QString)));  //send command key from textEdit to Scene
    QObject::connect(ui->graphicsView, SIGNAL(signalViewInit()), myScene, SLOT(slotSceneInit()), Qt::QueuedConnection); //try to get actual size of Scene не работает
    QObject::connect(ui->graphicsView, SIGNAL(signalViewInit()), this, SLOT(setSceneSize()), Qt::QueuedConnection);     //try to get actual size of Scene  не работает
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setSceneSize()
{
    myScene->setBasePoint(QPointF(10,ui->graphicsView->height()-10));

}

//void MainWindow::drawLineToggled(bool b)
//{
//    if (b)
//    {emit ui->actionLine->triggered();}
//}

//void MainWindow::drawRectToggled(bool b)
//{
//    if (b)
//    {emit ui->actionRectangle->triggered();}
//}

//void MainWindow::divideToggled(bool b)
//{
//    if (b)
//    {emit ui->actiondivide->triggered();}
//}

//void MainWindow::newSectionToggled(bool b)
//{
//    if (b)
//    {emit ui->actionNewSection->triggered();}
//}

void MainWindow::slotFitView()
{
    ui->graphicsView->fitInView(myScene->sceneRect(), Qt::KeepAspectRatio);
}

void MainWindow::slotCoordChanged(QPointF point)
{
    ui->statusBar->showMessage("x:"+ QString::number(point.x())+" y:"+ QString::number(point.y()));
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    //qDebug()<<"Draw mode return "<<myScene->getDrawMode();
    if(event->key()==Qt::Key_C)
    {
        if(myScene->getDrawMode())
        {
            emit signalKeyPressed(QKeySequence(event->key()).toString());
        }
    }
    QWidget::keyPressEvent(event);
}

