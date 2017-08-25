#include "aboutconfusionmatrixdialog.h"
#include "ui_aboutconfusionmatrixdialog.h"

AboutConfusionMatrixDialog::AboutConfusionMatrixDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutConfusionMatrixDialog)
{
    ui->setupUi(this);
}

AboutConfusionMatrixDialog::~AboutConfusionMatrixDialog()
{
    delete ui;
}
