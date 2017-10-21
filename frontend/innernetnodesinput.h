#ifndef INNERNETNODESINPUT_H
#define INNERNETNODESINPUT_H

#include <QWidget>
#include <QTableWidget>
#include <QSpinBox>

#include <backend/NeuralNet/NeuralNet.h>

namespace Ui {
class InnerNetNodesInput;
}

class InnerNetNodesInput : public QWidget
{
    Q_OBJECT

public:
    explicit InnerNetNodesInput(NeuralNet::NeuralNetParameters & n, QWidget *parent = 0);
    ~InnerNetNodesInput();

protected slots:
    void apply();
    void cancel();
    void updateCount();

private:
    Ui::InnerNetNodesInput *ui;

    std::vector<int> tempNodeCounts;

    int _innerNodesCount=0;
    NeuralNet::NeuralNetParameters & _nParams;

    void initializeBuffer();
    void fillBuffer();
    void buildTable();

    QSpinBox * getSpinBoxFromTable(const int & row);
};

#endif // INNERNETNODESINPUT_H
