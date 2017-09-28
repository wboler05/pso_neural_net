#include "maxmemorydialog.h"
#include "ui_maxmemorydialog.h"

MaxMemoryDialog::MaxMemoryDialog(const int &defaultBytes, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MaxMemoryDialog)
{
    ui->setupUi(this);

    initializeComboBox();
    setMaxBytes((double) defaultBytes);

    show();
}

MaxMemoryDialog::~MaxMemoryDialog()
{
    delete ui;
}

void MaxMemoryDialog::setMaxBytes(const double & v) {
    ui->maxBytes_dsb->setValue(
        convertBytes(v, Bytes, ui->byteSize_cb->currentData().value<WordType>())
    );
}

void MaxMemoryDialog::initializeComboBox() {
    ui->byteSize_cb->clear();

    ui->byteSize_cb->addItem("B", QVariant(Bytes));
    ui->byteSize_cb->addItem("KB", QVariant(KBytes));
    ui->byteSize_cb->addItem("MB", QVariant(MBytes));
    ui->byteSize_cb->addItem("GB", QVariant(GBytes));

    ui->byteSize_cb->setCurrentIndex(1);
    _dialogSettingType = KBytes;
}

double MaxMemoryDialog::convertBytes(double data, const WordType & fromType, const WordType & toType) {
    double dataB = convertToBytes(data, fromType);

    switch(toType) {
    case Bytes:
        return dataB;
        break;
    case KBytes:
        return dataB / 1024.0;
        break;
    case MBytes:
        return dataB / pow(1024.0, 2);
        break;
    case GBytes:
        return dataB / pow(1024.0, 3);
        break;
    default:
        return dataB;
        break;
    }
}

double MaxMemoryDialog::convertToBytes(double data, const WordType & type) {
    switch(type) {
    case Bytes:
        return data;
        break;
    case KBytes:
        return data*1024.0;
        break;
    case MBytes:
        return data*pow(1024, 2);
        break;
    case GBytes:
        return data*pow(1024, 3);
        break;
    default:
        return data;
    }
}

void MaxMemoryDialog::accepted() {
    WordType type = ui->byteSize_cb->currentData().value<WordType>();
    int bytes = (int) convertToBytes(ui->maxBytes_dsb->value(), type);
    emit maxBytesUpdated(bytes);
    close();
}

void MaxMemoryDialog::cancelled() {
    close();
}
