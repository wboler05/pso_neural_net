#include "slicenumberdialog.h"
#include "ui_slicenumberdialog.h"

SliceNumberDialog::SliceNumberDialog(const int & defaultSlices, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SliceNumberDialog)
{
    ui->setupUi(this);
    ui->slices_sb->setValue(defaultSlices);

    connect(ui->ok_btn, SIGNAL(clicked()), this, SLOT(accepted()));
    connect(ui->cancel_btn, SIGNAL(clicked()), this, SLOT(cancelled()));

    show();
}

SliceNumberDialog::~SliceNumberDialog()
{
    delete ui;
}

void SliceNumberDialog::accepted() {
    emit slicesUpdated(ui->slices_sb->value());
    close();
}

void SliceNumberDialog::cancelled() {
    close();
}
