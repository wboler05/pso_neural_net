#include "neuralnetplot.h"

#include <qwt_plot_curve.h>

NeuralNetPlot::NeuralNetPlot()
{
    setTitle(QwtText("Neural Net Plot"));

    QwtPlotCurve *curve1 = new QwtPlotCurve("Curve 1");
}

void NeuralNetPlot::replot() {

}
