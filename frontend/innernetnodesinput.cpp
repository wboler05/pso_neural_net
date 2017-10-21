#include "innernetnodesinput.h"
#include "ui_innernetnodesinput.h"

InnerNetNodesInput::InnerNetNodesInput(NeuralNet::NeuralNetParameters & n, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InnerNetNodesInput),
    _nParams(n)
{
    ui->setupUi(this);

    _innerNodesCount = static_cast<int>(_nParams.innerNetNodes.size());
    initializeBuffer();

    ui->innerNodeCount_dsb->setValue(static_cast<int>(_innerNodesCount));

    connect(ui->updateCount_btn, SIGNAL(clicked(bool)), this, SLOT(updateCount()));
    connect(ui->ok_btn, SIGNAL(clicked(bool)), this, SLOT(apply()));
    connect(ui->cancel_btn, SIGNAL(clicked(bool)), this, SLOT(cancel()));
    connect(ui->innerNodeCount_dsb, SIGNAL(valueChanged()), this, SLOT(updateCount()));

    buildTable();

    show();
}

InnerNetNodesInput::~InnerNetNodesInput()
{
    delete ui;
}

void InnerNetNodesInput::initializeBuffer() {
    tempNodeCounts.resize(_nParams.innerNetNodes.size());
    for (size_t i = 0; i < tempNodeCounts.size(); i++) {
        tempNodeCounts[i] = _nParams.innerNetNodes[i];
    }
}

void InnerNetNodesInput::updateCount() {
    fillBuffer();
    _innerNodesCount = ui->innerNodeCount_dsb->value();
    buildTable();
}

void InnerNetNodesInput::fillBuffer() {
    QTableWidget * table = ui->table;
    size_t rows = static_cast<size_t>(table->rowCount());
    for (size_t i = 0; i < rows; i++) {
        QSpinBox * box = getSpinBoxFromTable(static_cast<int>(i));

        if (i > tempNodeCounts.size()) {
            tempNodeCounts.push_back(0);
        } else {
            tempNodeCounts[i] = box->value();
        }
    }
}

QSpinBox * InnerNetNodesInput::getSpinBoxFromTable(const int & row) {
    return qobject_cast<QSpinBox *>(ui->table->cellWidget(row, 0));
}

void InnerNetNodesInput::buildTable() {
    QTableWidget * table = ui->table;

    table->clear();

    table->setColumnCount(1);
    table->setRowCount(_innerNodesCount);

    QStringList hHeader;
    hHeader.append("Inner Node Count");
    table->setHorizontalHeaderLabels(hHeader);

    for (size_t i = 0; i < static_cast<size_t>(_innerNodesCount); i++) {
        QSpinBox * spinBox = new QSpinBox();
        spinBox->setRange(1, 10000);

        if (i >= _nParams.innerNetNodes.size()) {
            spinBox->setValue(0);
        } else {
            spinBox->setValue(_nParams.innerNetNodes[i]);
        }

        table->setCellWidget(static_cast<int>(i), 0, spinBox);
    }
}

void InnerNetNodesInput::apply() {

    _nParams.innerNetNodes.resize(static_cast<size_t>(_innerNodesCount));
    _nParams.innerNets = _innerNodesCount;

    fillBuffer();

    for (size_t i = 0; i < static_cast<size_t>(_innerNodesCount); i++) {
        _nParams.innerNetNodes[i] = tempNodeCounts[i];
    }
    close();
}

void InnerNetNodesInput::cancel() {
    close();
}
