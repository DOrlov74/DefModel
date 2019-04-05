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
    QObject::connect(ui->drawLineButton, SIGNAL(toggled(bool)),this, SLOT(drawLineToggled(bool)));
    QObject::connect(ui->drawRectButton, SIGNAL(toggled(bool)),this, SLOT(drawRectToggled(bool)));
    QObject::connect(ui->divideButton, SIGNAL(toggled(bool)), this, SLOT(divideToggled(bool)));
    QObject::connect(ui->newButton, SIGNAL(toggled(bool)), this, SLOT(newSectionToggled(bool)));
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->drawLineButton, SLOT(setEnabled(bool)));
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->drawRectButton, SLOT(setEnabled(bool)));
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->newButton, SLOT(setDisabled(bool)));
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->divideButton, SLOT(setDisabled(bool)));
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->actionLine, SLOT(setEnabled(bool)));
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->actionRectangle, SLOT(setEnabled(bool)));
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->commandTextEdit, SLOT(slotDrawMode(bool)));
    QObject::connect(ui->actionLine, SIGNAL(triggered()), myScene, SLOT(setDrawLine()));
    QObject::connect(ui->actionLine, SIGNAL(triggered()), ui->commandTextEdit, SLOT(slotDrawLine()));
    QObject::connect(ui->actionRectangle, SIGNAL(triggered()), myScene, SLOT(setDrawRect()));
    QObject::connect(ui->actionRectangle, SIGNAL(triggered()), ui->commandTextEdit, SLOT(slotDrawRect()));
    QObject::connect(ui->actiondivide, SIGNAL(triggered()), ui->commandTextEdit, SLOT(slotDivide()));
    QObject::connect(myScene, SIGNAL(signalPointAdded(QPointF)), ui->commandTextEdit, SLOT(slotAddPoint(QPointF)));
    QObject::connect(this, SIGNAL(signalKeyPressed(QString)), ui->commandTextEdit, SLOT(slotAddKey(QString)));
    QObject::connect(this, SIGNAL(signalKeyPressed(QString)), myScene, SLOT(slotGetCommand(QString)));
    QObject::connect(ui->commandTextEdit, SIGNAL(signalCommand(QString)), myScene, SLOT(slotGetCommand(QString)));
    QObject::connect(ui->graphicsView, SIGNAL(signalViewInit()), myScene, SLOT(slotSceneInit()), Qt::QueuedConnection);// не работает
    QObject::connect(ui->graphicsView, SIGNAL(signalViewInit()), this, SLOT(setSceneSize()), Qt::QueuedConnection); // не работает
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setSceneSize()
{
    myScene->setBasePoint(QPointF(10,ui->graphicsView->height()-10));

}

void MainWindow::drawLineToggled(bool b)
{
    if (b)
    {emit ui->actionLine->triggered();}
}

void MainWindow::drawRectToggled(bool b)
{
    if (b)
    {emit ui->actionRectangle->triggered();}
}

void MainWindow::divideToggled(bool b)
{
    if (b)
    {emit ui->actiondivide->triggered();}
}

void MainWindow::newSectionToggled(bool b)
{
    if (b)
    {emit ui->actionNewSection->triggered();}
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

