#include "neuralnetplot.h"

#include <qwt_plot_curve.h>

NeuralNetPlot::NeuralNetPlot(QWidget *parent) :
    QwtPlot(parent)
{
    setTitle(QwtText("Neural Net Plot"));

    setAxisAutoScale(yLeft, true);
    setAxisAutoScale(xBottom, true);
}

void NeuralNetPlot::updateNodes() {
    if (_edges==nullptr) return;

    this->detachItems();

    const double lineThickness = 5;

    float minX = 0, maxX = 0;
    float minY = 0, maxY = 1;

    // Plot Nodes
    maxX = _edges->size();
    for (size_t i = 0; i < _edges->size(); i++) {

        double y_offset = 1.0f / (double) _edges->at(i).size();
        //maxY = std::max(maxY, (float)_edges->at(i).size());

        for (size_t j = 0; j < _edges->at(i).size(); j++) {

            qreal x = i;
            qreal y = y_offset * (double) j;

            QwtPlotMarker * mark = getNodeMarker(QPointF(i, y_offset * (double)j));
            mark->attach(this);

            // Edges
            double next_y_offset = 1.0f / (double) _edges->at(i).at(j).size();
            for (size_t k = 0; k < _edges->at(i).at(j).size(); k++) {
                qreal xf = i+1;
                qreal yf = next_y_offset * (double) k;

                QVector<QPointF> edgeData;
                edgeData.append(QPointF(x, y));
                edgeData.append(QPointF(xf, yf));

                QwtPlotCurve * newCurve = new QwtPlotCurve();
                newCurve->setSamples(edgeData);
                QColor curveColor = edgeColor(_edges->at(i).at(j).at(k));
                double lt = lineThickness * qAbs(_edges->at(i).at(j).at(k));
                //newCurve->setBrush(curveColor);
                newCurve->setPen(curveColor, lt);
                newCurve->attach(this);

            }
        }

        if (i == _edges->size() - 1) {

            y_offset = 1.0f / (double) _edges->at(i).at(0).size();
//            maxY = std::max(maxY, (float)_edges->at(i).size());

            for (size_t j = 0; j < _edges->at(i).at(0).size(); j++) {
                QwtPlotMarker * mark = getNodeMarker(QPointF(i+1, y_offset*(double)j));
                mark->attach(this);
            }
        }
    }
    setAxisScale(xBottom, minX-0.25, maxX+0.25);
    setAxisScale(yLeft, minY-0.05, maxY+0.05);
}

QColor NeuralNetPlot::edgeColor(double val) {
    if (val > 0) {
        return QColor(Qt::blue);
    } else if (val < 0) {
        return QColor(Qt::red);
    } else {
        return QColor(Qt::black);
    }
}

QwtPlotMarker * NeuralNetPlot::getNodeMarker(const QPointF & pos) {
    QwtSymbol *symbol = new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::green), QPen(Qt::green), QSize(5,5));
    QwtPlotMarker *mark = new QwtPlotMarker();
    mark->setSymbol(symbol);
    mark->setValue(pos);
    return mark;
}

void NeuralNetPlot::setEdges(NeuralNet::EdgeType * edges) {
    _edges = edges;
    updateNodes();
    replot();
}
