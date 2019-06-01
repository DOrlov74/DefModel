#include "infoform.h"
#include "ui_infoform.h"
#include <QButtonGroup>

InfoForm::InfoForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoForm)
{
    ui->setupUi(this);
    QObject::connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(slotApplyPressed()));
    QButtonGroup* bGroup=new QButtonGroup(this);
    bGroup->addButton(ui->stressMPaButton);
    bGroup->addButton(ui->stressKg_sm2Button);
    bGroup->addButton(ui->areaButton);
    bGroup->addButton(ui->strainButton);
    QObject::connect(ui->stressMPaButton, SIGNAL(toggled(bool)), this, SLOT(slotStressMPaSelected()));
    QObject::connect(ui->stressKg_sm2Button, SIGNAL(toggled(bool)), this, SLOT(slotStressKg_sm2Selected()));
    QObject::connect(ui->strainButton, SIGNAL(toggled(bool)), this, SLOT(slotStrainSelected()));
    QObject::connect(ui->areaButton, SIGNAL(toggled(bool)), this, SLOT(slotAreaSelected()));
}

InfoForm::~InfoForm()
{
    delete ui;
}

//void InfoForm::setValue(int i)
//{
//    ui->cProgressBar->setValue(i);
//}

void InfoForm::slotExportStart()
{
    ui->exportLabel->setText("Result is saving to result.xls. Please wait...");
}

void InfoForm::slotExportPercentChanged(int i)
{
    ui->cProgressBar->setValue(i);
}

void InfoForm::slotExportEnd()
{
    ui->exportLabel->setText("Rasult is saved successfully to result.xls");
}

void InfoForm::slotApplyPressed()
{
    emit signalApplyPressed(m_resultMode);
    ui->applyButton->setDisabled(true);
}

void InfoForm::slotStressMPaSelected()
{
    m_resultMode=1;
    ui->applyButton->setEnabled(true);
}

void InfoForm::slotStressKg_sm2Selected()
{
    m_resultMode=2;
    ui->applyButton->setEnabled(true);
}

void InfoForm::slotStrainSelected()
{
    m_resultMode=3;
    ui->applyButton->setEnabled(true);
}

void InfoForm::slotAreaSelected()
{
    m_resultMode=4;
    ui->applyButton->setEnabled(true);
}
