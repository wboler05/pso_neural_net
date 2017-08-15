#ifndef NEURALNETPLOT_H
#define NEURALNETPLOT_H

#include <QPointF>

#include <qwt_plot.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>

#include <vector>

class NeuralNetPlot : public QwtPlot
{
public:
    NeuralNetPlot(QWidget *parent=nullptr);

    void setEdges(std::vector<std::vector<std::vector<double>>> * edges);

public slots:
    void updateNodes();

private:
    std::vector<std::vector<std::vector<double>>> *_edges=nullptr;

    QColor edgeColor(double val);
    QwtPlotMarker * getNodeMarker(const QPointF & pos);
};

#endif // NEURALNETPLOT_H
