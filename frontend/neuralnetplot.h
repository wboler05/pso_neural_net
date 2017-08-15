#ifndef NEURALNETPLOT_H
#define NEURALNETPLOT_H

#include <qwt_plot.h>


class NeuralNetPlot : public QwtPlot
{
public:
    NeuralNetPlot();

public slots:
    void replot();
};

#endif // NEURALNETPLOT_H
