#include "innernetnodesinput.h"
#include "ui_innernetnodesinput.h"

InnerNetNodesInput::InnerNetNodesInput(NeuralNet::NeuralNetParameters & n, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InnerNetNodesInput),
    _nParams(n)
{
    ui->setupUi(this);

    _innerNodesCount = std::max((int)_nParams.innerNetNodes.size(), 1);
    ui->count_btn->setValue(_innerNodesCount);

    connect(ui->updateCount_btn, SIGNAL(clicked(bool)), this, SLOT(updateCount()));
    connect(ui->ok_btn, SIGNAL(clicked(bool)), this, SLOT(apply()));
    connect(ui->cancel_btn, SIGNAL(clicked(bool)), this, SLOT(cancel()));

    buildTable();

    show();
}

InnerNetNodesInput::~InnerNetNodesInput()
{
    delete ui;
}

void InnerNetNodesInput::updateCount() {
    _innerNodesCount = std::max((int)ui->count_btn->value(), 1);
    buildTable();
}

void InnerNetNodesInput::buildTable() {
    QTableWidget * table = ui->table;

    table->clear();

    table->setColumnCount(1);
    table->setRowCount(_innerNodesCount);

    QStringList hHeader;
    hHeader.append("Inner Node Count");
    table->setHorizontalHeaderLabels(hHeader);

    for (int i = 0; i < _innerNodesCount; i++) {
        QSpinBox * spinBox = new QSpinBox();
        spinBox->setRange(1, 10000);

        if (i >= _nParams.innerNetNodes.size()) {
            spinBox->setValue(0);
        } else {
            spinBox->setValue(_nParams.innerNetNodes[i]);
        }

        table->setCellWidget(i, 0, spinBox);
    }
}

void InnerNetNodesInput::apply() {
    QTableWidget * table = ui->table;

    _nParams.innerNetNodes.resize(_innerNodesCount);
    _nParams.innerNets = _innerNodesCount;

    for (int i = 0; i < _innerNodesCount; i++) {
        int val = qobject_cast<QSpinBox *>(table->cellWidget(i, 0))->value();
        _nParams.innerNetNodes[i] = val;
    }
    close();
}

void InnerNetNodesInput::cancel() {
    close();
}
