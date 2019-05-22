#include "excelinouthelper.h"
#include <QDebug>
#include <QFileInfo>
//#include <ActiveQt/QAxBase>
//#include <ActiveQt/QAxObject>
#include <QFileDialog>

ExcelInOutHelper::ExcelInOutHelper(QObject *parent) : QObject(parent)
{

}

ExcelInOutHelper::~ExcelInOutHelper()
{
    delete m_excel;
    delete m_workbooks;
    delete m_workbook;
    delete m_sheets;
    delete m_sheet;
    delete m_usedRange;
    delete m_rows;
    delete m_cols;
}

void ExcelInOutHelper::importPoints(QString fileName, int sheetNumber)
{
    m_excel = new QAxObject( "Excel.Application", nullptr );
    m_workbooks=m_excel->querySubObject("Workbooks");
    m_workbook=m_workbooks->querySubObject("Open(const QString&)", QFileInfo(fileName).absoluteFilePath());
    m_sheets=m_workbook->querySubObject("Worksheets");
    m_sheet=m_sheets->querySubObject("Item(int)", sheetNumber);
    m_usedRange=m_sheet->querySubObject("UsedRange");
    m_rows=m_usedRange->querySubObject("Rows");
    m_cols=m_usedRange->querySubObject("Columns");
    QFile file1("D://temp//sheet2.html");
        file1.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file1);
        out << m_sheet->generateDocumentation();
        file1.close();
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
    delete cell;
}

void ExcelInOutHelper::exportPoints(const QVector<QPointF>& vCPoints, const QVector<QPair<uint,QPointF>>& vRPoints)
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
    delete cell;
}

void ExcelInOutHelper::saveArea(const QVector<QVector<double>>& vCArea, const QVector<double>& vRArea)
{
    m_excel = new QAxObject( "Excel.Application", nullptr );
    m_excel->setProperty("DisplayAlerts", false);
    m_workbooks=m_excel->querySubObject("Workbooks");
    m_workbook=m_workbooks->querySubObject("Add");
    m_sheets=m_workbook->querySubObject("Worksheets");
    m_sheet=m_sheets->querySubObject("Item(int)", 1);
    int curRow=1;
    int curCol=1;
    QAxObject * cell = m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
    cell->setProperty("Value", "Concrete Area in m2:");
    for (int i=0; i<vCArea.size(); ++i)
    {
        for (int j=0; j<vCArea[i].size(); ++j)
        {
        ++curRow;
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
        cell->setProperty("Value", QVariant(vCArea[i][j]));
        }
        if (i<vCArea.size()-1)
        {curRow-=vCArea[i].size();}
        ++curCol;
    }
    ++curRow;
    curCol=1;
    cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
    cell->setProperty("Value", "Reinforcement Area in m2:");
    ++curRow;
    for (int i=0; i<vRArea.size(); ++i)
    {
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
        cell->setProperty("Value", QVariant(vRArea[i]));
        ++curCol;
    }
    QString fileName=QDir::currentPath()+"/report.xls";
    qDebug()<<fileName;
    m_workbook->dynamicCall("SaveAs(const QString&, QVariant)", fileName.replace("/", "\\"), -4143);
    m_workbook->dynamicCall("Close()");
    m_excel->dynamicCall("Quit()");
    delete cell;
}

void ExcelInOutHelper::saveCenterDist(const QVector<QVector<QPointF>>& vCDist, const QVector<QPointF>& vRDist)
{
    QString fileName= QDir::currentPath()+"/report.xls";
    m_excel = new QAxObject( "Excel.Application", nullptr );
    m_excel->setProperty("DisplayAlerts", false);
    m_workbooks=m_excel->querySubObject("Workbooks");
    m_workbook=m_workbooks->querySubObject("Open(const QString&)", QFileInfo(fileName).absoluteFilePath());
    m_sheets=m_workbook->querySubObject("Worksheets");
    m_sheet=m_sheets->querySubObject("Item(int)", 1);
    m_usedRange=m_sheet->querySubObject("UsedRange");
    m_rows=m_usedRange->querySubObject("Rows");
    m_cols=m_usedRange->querySubObject("Columns");
    int curRow=m_rows->property("Count").toInt()+1;
    int curCol=1;
    QAxObject * cell = m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
    cell->setProperty("Value", "Distant of concrete section from center point along X axis in m:");
    for (int i=0; i<vCDist.size(); ++i)
    {
        for (int j=0; j<vCDist[i].size(); ++j)
        {
        ++curRow;
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
        cell->setProperty("Value", QVariant(vCDist[i][j].x()));
        }
        if (i<vCDist.size()-1)
        {curRow-=vCDist[i].size();}
        ++curCol;
    }
    ++curRow;
    curCol=1;
    cell = m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
    cell->setProperty("Value", "Distant of concrete section from center point along Y axis in m:");
    for (int i=0; i<vCDist.size(); ++i)
    {
        for (int j=0; j<vCDist[i].size(); ++j)
        {
        ++curRow;
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
        cell->setProperty("Value", QVariant(vCDist[i][j].y()));
        }
        if (i<vCDist.size()-1)
        {curRow-=vCDist[i].size();}
        ++curCol;
    }
    ++curRow;
    curCol=1;
    cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
    cell->setProperty("Value", "Distant of Reinforcement bar from center point along X axis in m:");
    ++curRow;
    for (int i=0; i<vRDist.size(); ++i)
    {
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
        cell->setProperty("Value", QVariant(vRDist[i].x()));
        ++curCol;
    }
    ++curRow;
    curCol=1;
    cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
    cell->setProperty("Value", "Distant of Reinforcement bar from center point along Y axis in m:");
    ++curRow;
    for (int i=0; i<vRDist.size(); ++i)
    {
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
        cell->setProperty("Value", QVariant(vRDist[i].y()));
        ++curCol;
    }
    m_workbook->dynamicCall("SaveAs(const QString&, QVariant)", fileName.replace("/", "\\"), -4143);
    m_workbook->dynamicCall("Close()");
    m_excel->dynamicCall("Quit()");
    delete cell;
}

void ExcelInOutHelper::saveKElast(const QVector<QVector<double>>& vKbElast, const QVector<double>& vKrElast)
{
    QString fileName= QDir::currentPath()+"/report.xls";
    m_excel = new QAxObject( "Excel.Application", nullptr );
    m_excel->setProperty("DisplayAlerts", false);
    m_workbooks=m_excel->querySubObject("Workbooks");
    m_workbook=m_workbooks->querySubObject("Open(const QString&)", QFileInfo(fileName).absoluteFilePath());
    m_sheets=m_workbook->querySubObject("Worksheets");
    m_sheet=m_sheets->querySubObject("Item(int)", 1);
    m_usedRange=m_sheet->querySubObject("UsedRange");
    m_rows=m_usedRange->querySubObject("Rows");
    m_cols=m_usedRange->querySubObject("Columns");
    int curRow=m_rows->property("Count").toInt()+1;
    int curCol=1;
    QAxObject * cell = m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
    cell->setProperty("Value", "Coefficient of elasticy of concrete section:");
    for (int i=0; i<vKbElast.size(); ++i)
    {
        for (int j=0; j<vKbElast[i].size(); ++j)
        {
        ++curRow;
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
        cell->setProperty("Value", QVariant(vKbElast[i][j]));
        }
        if (i<vKbElast.size()-1)
        {curRow-=vKbElast[i].size();}
        ++curCol;
    }
    ++curRow;
    curCol=1;
    cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
    cell->setProperty("Value", "Coefficient of elasticy of Reinforcement bar:");
    ++curRow;
    for (int i=0; i<vKrElast.size(); ++i)
    {
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
        cell->setProperty("Value", QVariant(vKrElast[i]));
        ++curCol;
    }
    m_workbook->dynamicCall("SaveAs(const QString&, QVariant)", fileName.replace("/", "\\"), -4143);
    m_workbook->dynamicCall("Close()");
    m_excel->dynamicCall("Quit()");
    delete cell;
}

void ExcelInOutHelper::savevEb(const QVector<QVector<double>>& vEb)
{
    QString fileName= QDir::currentPath()+"/report.xls";
    m_excel = new QAxObject( "Excel.Application", nullptr );
    m_excel->setProperty("DisplayAlerts", false);
    m_workbooks=m_excel->querySubObject("Workbooks");
    m_workbook=m_workbooks->querySubObject("Open(const QString&)", QFileInfo(fileName).absoluteFilePath());
    m_sheets=m_workbook->querySubObject("Worksheets");
    m_sheet=m_sheets->querySubObject("Item(int)", 1);
    m_usedRange=m_sheet->querySubObject("UsedRange");
    m_rows=m_usedRange->querySubObject("Rows");
    m_cols=m_usedRange->querySubObject("Columns");
    int curRow=m_rows->property("Count").toInt()+1;
    int curCol=1;
    QAxObject * cell = m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
    cell->setProperty("Value", "Modulus of elasticy of concrete section in MPa:");
    for (int i=0; i<vEb.size(); ++i)
    {
        for (int j=0; j<vEb[i].size(); ++j)
        {
        ++curRow;
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
        cell->setProperty("Value", QVariant(vEb[i][j]/1000));
        }
        if (i<vEb.size()-1)
        {curRow-=vEb[i].size();}
        ++curCol;
    }
    m_workbook->dynamicCall("SaveAs(const QString&, QVariant)", fileName.replace("/", "\\"), -4143);
    m_workbook->dynamicCall("Close()");
    m_excel->dynamicCall("Quit()");
    delete cell;
}

void ExcelInOutHelper::saveStrain(const QVector<QVector<double>>& vCStrain, const QVector<double>& vRStrain)
{
    QString fileName= QDir::currentPath()+"/report.xls";
    m_excel = new QAxObject( "Excel.Application", nullptr );
    m_excel->setProperty("DisplayAlerts", false);
    m_workbooks=m_excel->querySubObject("Workbooks");
    m_workbook=m_workbooks->querySubObject("Open(const QString&)", QFileInfo(fileName).absoluteFilePath());
    m_sheets=m_workbook->querySubObject("Worksheets");
    m_sheet=m_sheets->querySubObject("Item(int)", 1);
    m_usedRange=m_sheet->querySubObject("UsedRange");
    m_rows=m_usedRange->querySubObject("Rows");
    m_cols=m_usedRange->querySubObject("Columns");
    int curRow=m_rows->property("Count").toInt()+1;
    int curCol=1;
    QAxObject * cell = m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
    cell->setProperty("Value", "Strain in concrete section x1000:");
    for (int i=0; i<vCStrain.size(); ++i)
    {
        for (int j=0; j<vCStrain[i].size(); ++j)
        {
        ++curRow;
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
        cell->setProperty("Value", QVariant(vCStrain[i][j]*1000));
        }
        if (i<vCStrain.size()-1)
        {curRow-=vCStrain[i].size();}
        ++curCol;
    }
    ++curRow;
    curCol=1;
    cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
    cell->setProperty("Value", "Strain in Reinforcement bar x1000:");
    ++curRow;
    for (int i=0; i<vRStrain.size(); ++i)
    {
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
        cell->setProperty("Value", QVariant(vRStrain[i]*1000));
        ++curCol;
    }
    m_workbook->dynamicCall("SaveAs(const QString&, QVariant)", fileName.replace("/", "\\"), -4143);
    m_workbook->dynamicCall("Close()");
    m_excel->dynamicCall("Quit()");
    delete cell;
}

void ExcelInOutHelper::saveStress(const QVector<QVector<double>>& vCStress, const QVector<double>& vRStress)
{
    QString fileName= QDir::currentPath()+"/report.xls";
    m_excel = new QAxObject( "Excel.Application", nullptr );
    m_excel->setProperty("DisplayAlerts", false);
    m_workbooks=m_excel->querySubObject("Workbooks");
    m_workbook=m_workbooks->querySubObject("Open(const QString&)", QFileInfo(fileName).absoluteFilePath());
    m_sheets=m_workbook->querySubObject("Worksheets");
    m_sheet=m_sheets->querySubObject("Item(int)", 1);
    m_usedRange=m_sheet->querySubObject("UsedRange");
    m_rows=m_usedRange->querySubObject("Rows");
    m_cols=m_usedRange->querySubObject("Columns");
    int curRow=m_rows->property("Count").toInt()+1;
    int curCol=1;
    QAxObject * cell = m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
    cell->setProperty("Value", "Stress in concrete section in MPa:");
    for (int i=0; i<vCStress.size(); ++i)
    {
        for (int j=0; j<vCStress[i].size(); ++j)
        {
        ++curRow;
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
        cell->setProperty("Value", QVariant(vCStress[i][j]/1000));
        }
        if (i<vCStress.size()-1)
        {curRow-=vCStress[i].size();}
        ++curCol;
    }
    ++curRow;
    curCol=1;
    cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
    cell->setProperty("Value", "Stress in Reinforcement bar in MPa:");
    ++curRow;
    for (int i=0; i<vRStress.size(); ++i)
    {
        cell=m_sheet->querySubObject("Cells(int,int)", curRow, curCol );
        cell->setProperty("Value", QVariant(vRStress[i]/1000));
        ++curCol;
    }
    m_workbook->dynamicCall("SaveAs(const QString&, QVariant)", fileName.replace("/", "\\"), -4143);
    m_workbook->dynamicCall("Close()");
    m_excel->dynamicCall("Quit()");
    delete cell;
}

QVector<QPointF> ExcelInOutHelper::getConcreteData()
{
    return m_concreteData;
}

QVector<QPair<uint, QPointF> > ExcelInOutHelper::getReinfData()
{
    return  m_reinfData;
}
