#ifndef NEURALNETPLOT_H
#define NEURALNETPLOT_H

#include <QPointF>

#include <qwt_plot.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>

#include <vector>

#include "backend/NeuralNet/NeuralNet.h"

class NeuralNetPlot : public QwtPlot
{
public:
    NeuralNetPlot(QWidget *parent=nullptr);

    void setState(NeuralNet::State *state, NeuralNet::Type t);

public slots:
    void updateNodes();

private:
    NeuralNet::State * _state=nullptr;
    NeuralNet::Type _netType = NeuralNet::Feedforward;

    QColor edgeColor(double val);
    QwtPlotMarker * getNodeMarker(const QPointF & pos);
};

#endif // NEURALNETPLOT_H
