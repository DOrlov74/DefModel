#ifndef INFOFORM_H
#define INFOFORM_H

#include <QWidget>
#include <QProgressDialog>

namespace Ui {
class InfoForm;
}

class InfoForm : public QWidget
{
    Q_OBJECT

    int m_resultMode=1;
    bool m_saveToExcel=false;

signals:
    void signalApplyPressed(int);
    void signalSaveToExcel(bool);

public:
    explicit InfoForm(QWidget *parent = nullptr);
    ~InfoForm();
//    void setValue(int);

public slots:
    void slotExportStart();
    void slotExportPercentChanged(int);
    void slotExportEnd();
    void slotApplyPressed();
    void slotStressMPaSelected();
    void slotStressKg_sm2Selected();
    void slotStrainSelected();
    void slotAreaSelected();
    void slotSaveToExcel(bool);

private:
    Ui::InfoForm *ui;
};

#endif // INFOFORM_H
