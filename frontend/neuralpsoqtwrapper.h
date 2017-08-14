#ifndef NEURALPSOQTWRAPPER_H
#define NEURALPSOQTWRAPPER_H

#include <QMainWindow>
#include <QWidget>
#include <QCoreApplication>

#include "backend/neuralpso.h"

class NeuralPsoQtWrapper : public QWidget, public NeuralPso
{
    Q_OBJECT
public:
    explicit NeuralPsoQtWrapper(PsoParams pp, NeuralNetParameters np, QWidget *parent = nullptr);

    void processEvents();
    void runTrainerQt();

signals:

public slots:
};

#endif // NEURALPSOQTWRAPPER_H
