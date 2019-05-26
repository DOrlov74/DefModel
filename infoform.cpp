#include "infoform.h"
#include "ui_infoform.h"

InfoForm::InfoForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoForm)
{
    ui->setupUi(this);
}

InfoForm::~InfoForm()
{
    delete ui;
}

void InfoForm::setValue(int i)
{
    ui->cProgressBar->setValue(i);
}
