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
    void openFile(QString, int=1);
    void saveFile(const QVector<QPointF>&, const QVector<QPair<uint,QPointF>>&);
    QVector<QPointF> getConcreteData();
    QVector<QPair<uint,QPointF>> getReinfData();

signals:

public slots:
};

#endif // EXCELINOUTHELPER_H