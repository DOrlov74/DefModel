#include "excelinouthelper.h"
#include <QDebug>
#include <QFileInfo>
//#include <ActiveQt/QAxBase>
//#include <ActiveQt/QAxObject>
#include <QFileDialog>

ExcelInOutHelper::ExcelInOutHelper(QObject *parent) : QObject(parent)
{

}

void ExcelInOutHelper::openFile(QString fileName, int sheetNumber)
{
    m_excel = new QAxObject( "Excel.Application", nullptr );
    m_workbooks=m_excel->querySubObject("Workbooks");
    m_workbook=m_workbooks->querySubObject("Open(const QString&)", QFileInfo(fileName).absoluteFilePath());
    m_sheets=m_workbook->querySubObject("Worksheets");
    m_sheet=m_sheets->querySubObject("Item(int)", sheetNumber);
    m_usedRange=m_sheet->querySubObject("UsedRange");
    m_rows=m_usedRange->querySubObject("Rows");
    m_cols=m_usedRange->querySubObject("Columns");
//    QFile file1("I://Dan//C++//DEV-Qt//Labs//sheet2.html");
//        file1.open(QIODevice::WriteOnly | QIODevice::Text);
//        QTextStream out(&file1);
//        out << m_sheet->generateDocumentation();
//        file1.close();
    int firstRow=m_usedRange->property("Row").toInt();
    int firstCol=m_usedRange->property("Column").toInt();
    int nRows=m_rows->property("Count").toInt();
    int nCols=m_cols->property("Count").toInt();
    qDebug()<<"first row "<<QString::number(firstRow);
    qDebug()<<"first column "<<QString::number(firstCol);
    qDebug()<<QString::number(nRows)<<"rows ";
    qDebug()<<QString::number(nCols)<<"columns in "<<fileName;
    QAxObject * cell = m_sheet->querySubObject("Cells(int,int)", firstRow, firstCol );
    QVariant value=cell->dynamicCall("Value()");
    bool isNumeric=false;
    QPointF curPoint;
    uint curDiam;
    if (value.toString()=="Concrete points:")
    {
        qDebug() << "there are concrete points: ";
        do
        {
            ++firstRow;
            cell = m_sheet->querySubObject("Cells(int,int)", firstRow, firstCol );
            value=cell->dynamicCall("Value()");
            value.toDouble(&isNumeric);
            if (isNumeric)
            {
                qDebug() << "cell in row "<< firstRow <<" and column "<< firstCol << "contains" << value.toString();
                curPoint.setX(value.toDouble());
            }
            cell = m_sheet->querySubObject("Cells(int,int)", firstRow, firstCol+1 );
            value=cell->dynamicCall("Value()");
            value.toDouble(&isNumeric);
            if (isNumeric)
            {
                qDebug() << "cell in row "<< firstRow <<" and column "<< firstCol+1 << "contains" << value.toString();
                curPoint.setY(value.toDouble());
                m_concreteData.append(curPoint);
            }
        }
        while (firstRow<=nRows&&isNumeric);
    }
    cell = m_sheet->querySubObject("Cells(int,int)", firstRow, firstCol );
    value=cell->dynamicCall("Value()");
    if (value.toString()=="Reinforcement points:")
    {
        qDebug() << "there are reinforcement points: ";
        do
        {
            ++firstRow;
            cell = m_sheet->querySubObject("Cells(int,int)", firstRow, firstCol );
            value=cell->dynamicCall("Value()");
            value.toDouble(&isNumeric);
            if (isNumeric)
            {
                qDebug() << "cell in row "<< firstRow <<" and column "<< firstCol << "contains" << value.toString();
                curDiam=value.toInt();
            }
            cell = m_sheet->querySubObject("Cells(int,int)", firstRow, firstCol+1 );
            value=cell->dynamicCall("Value()");
            value.toDouble(&isNumeric);
            if (isNumeric)
            {
                qDebug() << "cell in row "<< firstRow <<" and column "<< firstCol+1 << "contains" << value.toString();
                curPoint.setX(value.toDouble());
            }
            cell = m_sheet->querySubObject("Cells(int,int)", firstRow, firstCol+2 );
            value=cell->dynamicCall("Value()");
            value.toDouble(&isNumeric);
            if (isNumeric)
            {
                qDebug() << "cell in row "<< firstRow <<" and column "<< firstCol+2 << "contains" << value.toString();
                curPoint.setY(value.toDouble());
                m_reinfData.append(QPair<uint, QPointF>(curDiam,curPoint));
            }
        }
        while (firstRow<=nRows&&isNumeric);
    }
    m_workbook->dynamicCall("Close()");
    m_excel->dynamicCall("Quit()");
}

void ExcelInOutHelper::saveFile(const QVector<QPointF>& vCPoints, const QVector<QPair<uint,QPointF>>& vRPoints)
{
    m_excel = new QAxObject( "Excel.Application", nullptr );
    m_excel->setProperty("DisplayAlerts", false);
    m_workbooks=m_excel->querySubObject("Workbooks");
    m_workbook=m_workbooks->querySubObject("Add");
    m_sheets=m_workbook->querySubObject("Worksheets");
    m_sheet=m_sheets->querySubObject("Item(int)", 1);
    int curRow=1;
    QAxObject * cell = m_sheet->querySubObject("Cells(int,int)", curRow, 1 );
    cell->setProperty("Value", "Concrete points:");
    for (int i=0; i<vCPoints.size(); ++i)
    {
        ++curRow;
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, 1 );
        cell->setProperty("Value", QVariant(vCPoints[i].x()));
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, 2 );
        cell->setProperty("Value", QVariant(vCPoints[i].y()));
    }
    ++curRow;
    cell=m_sheet->querySubObject("Cells(int,int)", curRow, 1 );
    cell->setProperty("Value", "Reinforcement points:");
    for (int i=0; i<vRPoints.size(); ++i)
    {
        ++curRow;
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, 1 );
        cell->setProperty("Value", QVariant(vRPoints[i].first));
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, 2 );
        cell->setProperty("Value", QVariant(vRPoints[i].second.x()));
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, 3 );
        cell->setProperty("Value", QVariant(vRPoints[i].second.y()));
    }
    QString fileName=QFileDialog::getSaveFileName(nullptr, "Save file with data", "data.xls", "excel(*.xls *.xlsx)");
    qDebug()<<fileName;
    m_workbook->dynamicCall("SaveAs(const QString&, QVariant)", fileName.replace("/", "\\"), -4143);
    m_workbook->dynamicCall("Close()");
    m_excel->dynamicCall("Quit()");
}

QVector<QPointF> ExcelInOutHelper::getConcreteData()
{
    return m_concreteData;
}

QVector<QPair<uint, QPointF> > ExcelInOutHelper::getReinfData()
{
    return  m_reinfData;
}
