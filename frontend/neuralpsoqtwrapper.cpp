#include "neuralpsoqtwrapper.h"

NeuralPsoQtWrapper::NeuralPsoQtWrapper(PsoParams pp, NeuralNetParameters np, QWidget *parent) :
    QWidget(parent),
    NeuralPso(pp, np)
{
}

void NeuralPsoQtWrapper::processEvents() {
    qApp->processEvents();
}

void NeuralPsoQtWrapper::runTrainerQt() {
    run();

    printGB();

    _neuralNet->setWeights(&gb()->_x);
}
