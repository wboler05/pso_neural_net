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

    size_t _innerNodesCount=0;
    NeuralNet::NeuralNetParameters & _nParams;

    void buildTable();
};

#endif // INNERNETNODESINPUT_H
