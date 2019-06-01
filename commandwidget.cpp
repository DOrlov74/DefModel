#include "commandwidget.h"
#include <QDebug>
//#include <QKeyEvent>
#include <QRegularExpression>

commandWidget::commandWidget(QWidget *parent) : QTextEdit(parent)
{
    this->append(">");
    QObject::connect(this, SIGNAL(textChanged()), this, SLOT(slotGetText()),Qt::QueuedConnection);
}

void commandWidget::slotDrawLine()
{
    this->append("Line> enter first point >");
    m_waitPoint=true;
    m_strPos=this->toPlainText().size();
}

void commandWidget::slotDrawRect()
{
    this->append("Rect> enter first point >");
    m_waitPoint=true;
    m_strPos=this->toPlainText().size();
}

void commandWidget::slotDrawPoint()
{
    this->append("Point> enter point >");
    m_waitPoint=true;
    m_drawPointMode=true;
    m_strPos=this->toPlainText().size();
}

void commandWidget::slotAddPoint(QPointF point)
{
    if (m_waitPoint)
    {
        if (m_drawPointMode)
        {
            this->append("x: "+QString::number(point.x())+" y: "+QString::number(point.y())+"> enter next point or 'd' for done >");
        }
        else if (n_points>1)
        {
            this->append("x: "+QString::number(point.x())+" y: "+QString::number(point.y())+"> enter next point or 'c' to close >");
            m_drawLineMode=true;
        }
        else
        {
            this->append("x: "+QString::number(point.x())+" y: "+QString::number(point.y())+"> enter next point >");
        }
    }
    else {
        this->append("x: "+QString::number(point.x())+" y: "+QString::number(point.y())+" >");
    }
    ++n_points;
    m_strPos=this->toPlainText().size();
}

void commandWidget::slotAddKey(QString str)
{
    if (str=="c"||str=="C")
    {
        this->append("close>");
        m_drawLineMode=false;
         n_points=0;
    }
    else if (str=="d"||str=="D")
    {
        this->append("done>");
        m_drawPointMode=false;
         n_points=0;
    }
    else
    {
     this->append(str);
    }
    m_strPos=this->toPlainText().size();
}

void commandWidget::slotGetText()
{
    m_strCommand=this->toPlainText().mid(m_strPos);
    qDebug()<<m_strCommand;
    if (m_waitPoint)
    {
        QRegularExpression reg("[+-]?([0-9]*[.])?[0-9]+,[+-]?([0-9]*[.])?[0-9]+\\n");
        QRegularExpressionMatch match=reg.match(m_strCommand);
        if (match.hasMatch())
        {
            qDebug()<<"filtered command: "<<match.captured(0);
            m_strCommand=match.captured(0);
            m_strCommand.chop(1);
            emit signalCommand(m_strCommand);
            m_strPos=this->toPlainText().size();
        }
    }
    if (m_drawPointMode||m_drawLineMode)
    {
        QRegularExpression reg3("[d|c]+\\n", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match=reg3.match(m_strCommand);
        if (match.hasMatch())
        {
            qDebug()<<"filtered command: "<<match.captured(0);
            m_strCommand=match.captured(0);
            m_strCommand.chop(1);
            emit signalCommand(m_strCommand);
            m_strPos=this->toPlainText().size();
        }
    }
    if (m_DivideMode)
    {
        QRegularExpression reg2("[0-9]+\\n");
        QRegularExpressionMatch match2=reg2.match(m_strCommand);
        if (match2.hasMatch())
        {
            qDebug()<<"filtered number: "<<match2.captured(0);
            m_strCommand=match2.captured(0);
            m_strCommand.chop(1);
            //emit signalCommand(m_strCommand);
            if (m_waitX)
            {
                m_waitX=false;
                m_waitY=true;
                emit signalDivideX(m_strCommand.toInt());
                this->append("Divide> Enter number of divisions along Y axis:<10>");
            }
            else if (m_waitY)
            {
                m_waitY=false;
                m_DivideMode=false;
                emit signalDivideY(m_strCommand.toInt());
                this->append("> ");
            }
            m_strPos=this->toPlainText().size();
        }
        else if (m_strCommand=="\n")
        {
            //emit signalCommand("10");
            if (m_waitX)
            {
                m_waitX=false;
                m_waitY=true;
                emit signalDivideX(10);
                this->append("Divide> Enter number of divisions along Y axis:<10>");
            }
            else if (m_waitY)
            {
                m_waitY=false;
                m_DivideMode=false;
                emit signalDivideY(10);
                this->append("> ");
            }
            m_strPos=this->toPlainText().size();
        }
    }
}

void commandWidget::slotDivide()
{
    m_DivideMode=true;
    this->append("Divide> Enter number of divisions along X axis:<10>");
    m_strPos=this->toPlainText().size();
    m_waitX=true;
}

void commandWidget::slotDrawMode(bool b)
{
    m_waitPoint=b;
}


//void commandWidget::keyPressEvent(QKeyEvent *event)
//{
//    if(event->key()==Qt::Key_C||event->key()==Qt::Key_D)
//    {
//        if(m_drawPointMode)
//        {
//            slotAddKey(QKeySequence(event->key()).toString());
//        }
//    }
//    QWidget::keyPressEvent(event);
//}
