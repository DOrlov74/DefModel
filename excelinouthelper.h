#ifndef EXCELINOUTHELPER_H
#define EXCELINOUTHELPER_H

#include <QObject>
#include <ActiveQt/QAxBase>
#include <ActiveQt/QAxObject>
#include <QVector>
#include <QPointF>

class ExcelInOutHelper : public QObject
{
    Q_OBJECT
    QAxObject* m_excel;
    QAxObject* m_workbooks;
    QAxObject* m_workbook;
    QAxObject* m_sheets;
    QAxObject* m_sheet;
    QAxObject* m_usedRange;
    QAxObject* m_rows;
    QAxObject* m_cols;
    QVariantList m_data;
    QVector<QPointF> m_concreteData;
    QVector<QPair<uint,QPointF>> m_reinfData;

public:
    explicit ExcelInOutHelper(QObject *parent = nullptr);
    ~ExcelInOutHelper();
    void importPoints(QString, int=1);
    void exportPoints(const QVector<QPointF>&, const QVector<QPair<uint,QPointF>>&);
    void saveArea(const QVector<QVector<double>>&, const QVector<double>&);
    void saveCenterDist(const QVector<QVector<QPointF>>&, const QVector<QPointF>&);
    void saveKElast(const QVector<QVector<double>>&, const QVector<double>&);
    void savevEb(const QVector<QVector<double>>&);
    void saveStrain(const QVector<QVector<double>>&, const QVector<double>&);
    void saveStress(const QVector<QVector<double>>&, const QVector<double>&);
    QVector<QPointF> getConcreteData();
    QVector<QPair<uint,QPointF>> getReinfData();

signals:

public slots:
};

#endif // EXCELINOUTHELPER_H
