#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "scene.h"
#include <QKeyEvent>
#include <QKeySequence>
#include <QDebug>
#include <QIntValidator>
#include <QDoubleValidator>
//#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    myScene= new Scene(this);
    ui->graphicsView->setScene(myScene);
    //myScene->setSceneRect(0,0,ui->graphicsView->width(),ui->graphicsView->height());
    //qDebug()<<"scene width: "<< ui->graphicsView->width()<<"scene height: "<<ui->graphicsView->height();
    //myScene->setBasePoint(QPointF(10,myScene->height()-10));
    QDoubleValidator* validator=new QDoubleValidator(this);
    ui->NLineEdit->setValidator(validator);
    ui->MyLineEdit->setValidator(validator);
    ui->MxLineEdit->setValidator(validator);
    ui->actionNewSection->setDisabled(true);
    ui->actiondivide->setDisabled(true);
    ui->actionCalculate->setDisabled(true);
    QCursor cursor;
    cursor.setShape(Qt::CrossCursor);
    ui->graphicsView->setCursor(cursor);
    QObject::connect(ui->drawLineButton, SIGNAL(clicked()),ui->actionLine, SLOT(trigger()));          //drawline event
    QObject::connect(ui->drawRectButton, SIGNAL(clicked()),ui->actionRectangle, SLOT(trigger()));          //drawRectangle event
    QObject::connect(ui->divideButton, SIGNAL(clicked()), ui->actiondivide, SLOT(trigger()));             //divide section event
    QObject::connect(ui->newButton, SIGNAL(clicked()), ui->actionNewSection, SLOT(trigger()));            // new section event
    QObject::connect(ui->drawPointButton, SIGNAL(clicked()), ui->actionPoint, SLOT(trigger()));         // draw point event
    QObject::connect(ui->newRButton, SIGNAL(clicked()), ui->actionNewReinf, SLOT(trigger()));         // new reinforcement event
    QObject::connect(ui->calculateButton, SIGNAL(clicked()), ui->actionCalculate, SLOT(trigger()));         // calculate event
    QObject::connect(ui->NLineEdit, SIGNAL(textChanged(QString)), myScene, SLOT(slotSetN(QString)));        //send N value to Scene
    QObject::connect(ui->MxLineEdit, SIGNAL(textChanged(QString)), myScene, SLOT(slotSetMx(QString)));      //send Mx value to Scene
    QObject::connect(ui->MyLineEdit, SIGNAL(textChanged(QString)), myScene, SLOT(slotSetMy(QString)));      //send My value to Scene
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
    QObject::connect(myScene, SIGNAL(signalSceneCleared(bool)), ui->divideButton, SLOT(setDisabled(bool)));
    QObject::connect(myScene, SIGNAL(signalSceneCleared(bool)), ui->actiondivide, SLOT(setDisabled(bool)));
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->actionLine, SLOT(setEnabled(bool)));        //enable/disable drawLine action
    QObject::connect(myScene, SIGNAL(signalSceneCleared(bool)), ui->actionLine, SLOT(setEnabled(bool)));
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->actionRectangle, SLOT(setEnabled(bool)));   //enable/disable drawRectangle action
    QObject::connect(myScene, SIGNAL(signalSceneCleared(bool)), ui->actionRectangle, SLOT(setEnabled(bool)));
    QObject::connect(myScene, SIGNAL(signalDrawMode(bool)), ui->commandTextEdit, SLOT(slotDrawMode(bool))); //send a signal of changing mode to textEdit
    QObject::connect(myScene, SIGNAL(signalCoordChanged(QPointF)), this, SLOT(slotCoordChanged(QPointF)));  //get actual coordinats from Scene
    QObject::connect(myScene, SIGNAL(signalSceneDivided(bool)), ui->drawPointButton, SLOT(setEnabled(bool)));     //enable draw point Button
    QObject::connect(myScene, SIGNAL(signalSceneDivided(bool)), ui->actionPoint, SLOT(setEnabled(bool)));
    QObject::connect(myScene, SIGNAL(signalReinfDone(bool)), ui->drawPointButton, SLOT(setDisabled(bool)));     //disable draw point Button
    QObject::connect(myScene, SIGNAL(signalReinfDone(bool)), ui->actionPoint, SLOT(setDisabled(bool)));
    QObject::connect(myScene, SIGNAL(signalReinfDone(bool)), ui->newRButton, SLOT(setEnabled(bool)));     //enable new reinforcement Button
    QObject::connect(myScene, SIGNAL(signalReinfDone(bool)), ui->actionNewReinf, SLOT(setEnabled(bool)));
    QObject::connect(myScene, SIGNAL(signalForcesDone(bool)), ui->calculateButton, SLOT(setEnabled(bool)));     //enable calculate Button
    QObject::connect(myScene, SIGNAL(signalForcesDone(bool)), ui->actionCalculate, SLOT(setEnabled(bool)));
    QObject::connect(ui->actionFit_to_section, SIGNAL(triggered()), myScene, SLOT(slotFitView()));             //send command fit view to scene
    //QObject::connect(myScene, SIGNAL(signalFitView()), this, SLOT(slotFitView()));                          //send command fit view to graphics view
    QObject::connect(ui->actionZoom_in, SIGNAL(triggered()), this, SLOT(slotZoomIn()));
    QObject::connect(ui->actionZoom_out, SIGNAL(triggered()), this, SLOT(slotZoomOut()));
    QObject::connect(ui->actionPan, SIGNAL(toggled(bool)), ui->graphicsView, SLOT(slotPan(bool)));
    QObject::connect(ui->actionLine, SIGNAL(triggered()), myScene, SLOT(setDrawLine()));                    //send command drawLine to Scene
    QObject::connect(ui->actionLine, SIGNAL(triggered()), ui->commandTextEdit, SLOT(slotDrawLine()));       //send command drawLine to textEdit
    QObject::connect(ui->actionRectangle, SIGNAL(triggered()), myScene, SLOT(setDrawRect()));               //send command drawRectangle to Scene
    QObject::connect(ui->actionRectangle, SIGNAL(triggered()), ui->commandTextEdit, SLOT(slotDrawRect()));  //send command drawRectangle to textEdit
    QObject::connect(ui->actionPoint, SIGNAL(triggered()), myScene, SLOT(setDrawPoint()));                  //send command drawPoint to Scene
    QObject::connect(ui->actionPoint, SIGNAL(triggered()), ui->commandTextEdit, SLOT(slotDrawPoint()));     //send command drawPoint to textEdit
    QObject::connect(ui->actiondivide, SIGNAL(triggered()), ui->commandTextEdit, SLOT(slotDivide()));       //send command divide to textEdit
    QObject::connect(ui->commandTextEdit, SIGNAL(signalDivideX(uint)), myScene, SLOT(slotDivideX(uint)));   //send number of divisions on axis X to Scene
    QObject::connect(ui->commandTextEdit, SIGNAL(signalDivideY(uint)), myScene, SLOT(slotDivideY(uint)));   //send number of divisions on axis Y to Scene
    QObject::connect(ui->actionNewSection, SIGNAL(triggered()), myScene, SLOT(slotNewSection()));           //send command new section to Scene
    QObject::connect(ui->actionNewReinf, SIGNAL(triggered()), myScene, SLOT(slotNewReinf()));               //send command new reinforcement to Scene
    QObject::connect(ui->actionCalculate, SIGNAL(triggered()), myScene, SLOT(slotCalculate()));                     //send command calculate to Scene
    QObject::connect(myScene, SIGNAL(signalReinfCleared(bool)), ui->drawPointButton, SLOT(setEnabled(bool)));       //enable draw Point button
    QObject::connect(myScene, SIGNAL(signalReinfCleared(bool)), ui->newRButton, SLOT(setDisabled(bool)));           //disable new reinforcement button
    QObject::connect(myScene, SIGNAL(signalReinfCleared(bool)), ui->actionPoint, SLOT(setEnabled(bool)));           //enable draw Point action
    QObject::connect(myScene, SIGNAL(signalReinfCleared(bool)), ui->actionNewReinf, SLOT(setDisabled(bool)));       //disable new reinforcement action
    QObject::connect(myScene, SIGNAL(signalReinfCleared(bool)), ui->calculateButton, SLOT(setDisabled(bool)));       //disable calculate button
    QObject::connect(myScene, SIGNAL(signalReinfCleared(bool)), ui->actionCalculate, SLOT(setDisabled(bool)));       //disable calculate action
    //QObject::connect(myScene, SIGNAL(signalGetRDiameter(int)), ui->diameterSpinBox, SLOT(setValue(int)));         //set current diameter in spinBox //не работает
    QObject::connect(ui->diameterSpinBox, SIGNAL(valueChanged(int)), myScene, SLOT(slotSetRDiameter(int)));         //get current diameter from spinBox
    QObject::connect(myScene, SIGNAL(signalPointAdded(QPointF)), ui->commandTextEdit, SLOT(slotAddPoint(QPointF))); //send added point to textEdit
    QObject::connect(this, SIGNAL(signalKeyPressed(QString)), ui->commandTextEdit, SLOT(slotAddKey(QString)));      //send command key from mainWidget to textEdit
    QObject::connect(this, SIGNAL(signalKeyPressed(QString)), myScene, SLOT(slotGetCommand(QString)));              //send command key from mainWidget to Scene
    QObject::connect(ui->commandTextEdit, SIGNAL(signalCommand(QString)), myScene, SLOT(slotGetCommand(QString)));  //send command key from textEdit to Scene
    QObject::connect(ui->graphicsView, SIGNAL(signalViewInit()), myScene, SLOT(slotSceneInit()), Qt::QueuedConnection); //try to get actual size of Scene не работает
    QObject::connect(ui->graphicsView, SIGNAL(signalViewInit()), this, SLOT(setSceneSize()), Qt::QueuedConnection);     //try to get actual size of Scene  не работает
    ui->diameterSpinBox->setValue(myScene->getCurrDiam());
    QObject::connect(ui->actionOpen, SIGNAL(triggered()), myScene, SLOT(slotImportPoints()));
    QObject::connect(ui->actionSave, SIGNAL(triggered()), myScene, SLOT(slotExportPoints()));
    QObject::connect(ui->concreteClassComboBox, SIGNAL(currentIndexChanged(QString)),this, SLOT(slotCClassChanged(QString)));       //send concrete class to slot
    QObject::connect(this, SIGNAL(signalSetEb(double)),myScene, SLOT(slotSetEb(double)));                                           //send concrete modulus of elasticity to my scene
    QObject::connect(this, SIGNAL(signalSetRb(double)),myScene, SLOT(slotSetRb(double)));                                           //send concrete compressive strength to my scene
    QObject::connect(this, SIGNAL(signalSetRbt(double)),myScene, SLOT(slotSetRbt(double)));                                         //send concrete tensile strength to my scene
    fillCClasses();                                                                                                                 //fill container with concrete classes
    fillCCombobox();                                                                                                                //fill combobox with concrete classes
    QObject::connect(ui->reinforcementClassComboBox, SIGNAL(currentIndexChanged(QString)),this, SLOT(slotRClassChanged(QString)));  //send reinforcement class to slot
    QObject::connect(this, SIGNAL(signalSetEs(double)),myScene, SLOT(slotSetEs(double)));                                           //send reinforcement modulus of elasticity to my scene
    QObject::connect(this, SIGNAL(signalSetRs(double)),myScene, SLOT(slotSetRs(double)));                                           //send reinforcement tensile strength to my scene
    fillRClasses();                                                                                                                 //fill container with reinforcement classes
    fillRCombobox();                                                                                                                //fill combobox with reinforcement classes
    QObject::connect(myScene, SIGNAL(signalCalcStart()), this, SLOT(slotCalcStart()));                          //send start calculation signal to mainwindow
    QObject::connect(myScene, SIGNAL(signalPercentChanged(int)), this, SLOT(slotPercentChanged(int)));          //send porgress percentage to mainwindow
    QObject::connect(myScene, SIGNAL(signalCalcEnd(bool)), this, SLOT(slotCalcEnd(bool)));                              //send end calculation signal to mainwindow
    myInfo=new InfoForm();
    myInfo->setWindowFlag(Qt::WindowStaysOnTopHint);
    myInfo->setWindowModality(Qt::NonModal);
    myInfo->setWindowTitle("Calculation result");
    QObject::connect(myScene, SIGNAL(signalExportStart()), myInfo, SLOT(slotExportStart()));                          //send start export to excel signal to info window
    QObject::connect(myScene, SIGNAL(signalExportPercentChanged(int)), myInfo, SLOT(slotExportPercentChanged(int)));          //send porgress percentage to info window
    QObject::connect(myScene, SIGNAL(signalExportEnd()), myInfo, SLOT(slotExportEnd()));                              //send end export to excel signal to info window
    QObject::connect(myInfo, SIGNAL(signalApplyPressed(int)), myScene, SLOT(slotApplyPressed(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
    //delete myInfo;
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

void MainWindow::slotZoomIn()
{
    ui->graphicsView->scale(1.25,1.25);
}

void MainWindow::slotZoomOut()
{
    ui->graphicsView->scale(0.8,0.8);
}

void MainWindow::slotCoordChanged(QPointF point)
{
    ui->statusBar->showMessage("x:"+ QString::number(point.x())+" y:"+ QString::number(point.y()));
}

void MainWindow::slotCClassChanged(QString str)
{
    emit signalSetEb(ui->concreteClassComboBox->currentData().value<QVector<double>>()[0]);
    emit signalSetRb(ui->concreteClassComboBox->currentData().value<QVector<double>>()[1]);
    emit signalSetRbt(ui->concreteClassComboBox->currentData().value<QVector<double>>()[2]);
    qDebug()<<"in slot concrete class changed to "<<str;
}

void MainWindow::slotRClassChanged(QString str)
{
    emit signalSetEs(ui->reinforcementClassComboBox->currentData().value<QPair<double,double>>().first);
    emit signalSetRs(ui->reinforcementClassComboBox->currentData().value<QPair<double,double>>().second);
    qDebug()<<"in slot reinforcement class changed to "<<str;
}

void MainWindow::slotCalcStart()
{
    myProgress= new QProgressDialog("Calculation in progress...","Cancel",0,100,this);

    myProgress->show();
}

void MainWindow::slotPercentChanged(int i)
{
    myProgress->setValue(i);
}

void MainWindow::slotCalcEnd(bool b)
{
    myProgress->hide();
    if (b)
    {
        myInfo->show();
    }
}

void MainWindow::fillCClasses()
{
    m_cClasses.insert("B7,5",QVector<double>{16000,4.5,0.48});
    m_cClasses.insert("B10",QVector<double>{19000,6,0.56});
    m_cClasses.insert("B12,5",QVector<double>{21500,7.5,0.66});
    m_cClasses.insert("B15",QVector<double>{24000,8.5,0.75});
    m_cClasses.insert("B20",QVector<double>{27500,11.5,0.9});
    m_cClasses.insert("B25",QVector<double>{30000,14.5,1.05});
    m_cClasses.insert("B30",QVector<double>{32500,17,1.15});
    m_cClasses.insert("B35",QVector<double>{34500,19.5,1.3});
    m_cClasses.insert("B40",QVector<double>{36000,22,1.4});
    m_cClasses.insert("B45",QVector<double>{37000,25,1.5});
    m_cClasses.insert("B50",QVector<double>{38000,27.5,1.6});
    m_cClasses.insert("B55",QVector<double>{39000,30,1.7});
    m_cClasses.insert("B60",QVector<double>{39500,33,1.8});
}

void MainWindow::fillCCombobox()
{
    for (QMap<QString,QVector<double>>::const_iterator it=m_cClasses.constBegin(); it!=m_cClasses.constEnd(); ++it)
    {
        ui->concreteClassComboBox->addItem(it.key(),QVariant::fromValue(it.value()));
    }
}

void MainWindow::fillRClasses()
{
    m_rClasses.insert("A240",QPair<double,double>(210000,210));
    m_rClasses.insert("A400",QPair<double,double>(200000,350));
}

void MainWindow::fillRCombobox()
{
    for (QMap<QString,QPair<double,double>>::const_iterator it=m_rClasses.constBegin(); it!=m_rClasses.constEnd(); ++it)
    {
        ui->reinforcementClassComboBox->addItem(it.key(),QVariant::fromValue(it.value()));
    }
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    //qDebug()<<"Draw mode return "<<myScene->getDrawMode();
    if(event->key()==Qt::Key_C||event->key()==Qt::Key_D)
    {
        if(myScene->getDrawMode())
        {
            emit signalKeyPressed(QKeySequence(event->key()).toString());
        }
    }
    QWidget::keyPressEvent(event);
}

