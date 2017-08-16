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

    void setEdges(NeuralNet::EdgeType *edges);

public slots:
    void updateNodes();

private:
    NeuralNet::EdgeType * _edges=nullptr;

    QColor edgeColor(double val);
    QwtPlotMarker * getNodeMarker(const QPointF & pos);
};

#endif // NEURALNETPLOT_H
