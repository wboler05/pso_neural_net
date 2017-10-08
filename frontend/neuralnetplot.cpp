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
    maxX = _edges->size()-1;
    for (size_t i = 0; i < _edges->size() - 1; i++) {
        if (_edges->at(i).size() > 0) {
            qreal y_offset = 1.0 / static_cast<double>(_edges->at(i).size());
            //maxY = std::max(maxY, (float)_edges->at(i).size());

            for (size_t j = 0; j < _edges->at(i).size(); j++) {

                qreal x = i;
                qreal y = y_offset * static_cast<qreal>(j);

                QwtPlotMarker * mark = getNodeMarker(QPointF(x, y));
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

            if (i == _edges->size() - 2) {
                if (_edges->at(i).size() > 0) {
                    if (_edges->at(i).at(0).size() > 0) {
                        y_offset = 1.0f / static_cast<double>(_edges->at(i).at(0).size());
            //            maxY = std::max(maxY, (float)_edges->at(i).size());

                        for (size_t j = 0; j < _edges->at(i).at(0).size(); j++) {
                            QwtPlotMarker * mark = getNodeMarker(QPointF(i+1, y_offset*(double)j));
                            mark->attach(this);
                        }
                    }
                }
            }
        }
    }

    // Draw Recurrent Loops
    if (_netType == NeuralNet::Recurrent) {
        int it = _edges->size() - 1;
        const qreal rx = 0.05;
        const qreal ry = 0.03;
        const qreal rad_res = 360;
        qreal radStep = 360.0 / (2.0 * M_PI * rad_res);
        qreal maxArc = (2.0 * M_PI) * ( 300.0 / 360.0 );

        for (int j = 0; j < _edges->at(it).size(); j++) {

            double y_offset = 1.0f / (double) _edges->at(it)[j].size();

            for (int k = 0; k < _edges->at(it)[j].size(); k++) {
                qreal x = j+1;
                qreal y = y_offset * (double) k;

                QVector<QPointF> edgeData;
                QwtPlotCurve * newCurve = new QwtPlotCurve();
                QColor curveColor = edgeColor(_edges->at(it)[j][k]);
                double lt = lineThickness * qAbs(_edges->at(it)[j][k]);
                newCurve->setPen(curveColor, lt);

                for (qreal rad = 0; rad < maxArc; rad += radStep) {
                    qreal x_i = -rx * cos(rad) + x;
                    qreal y_i =  ry * (sin(rad) + 1) + y;

                    edgeData.append(QPointF(x_i, y_i));
                }

                newCurve->setSamples(edgeData);
                newCurve->attach(this);
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

void NeuralNetPlot::setEdges(NeuralNet::CombEdgeType * edges, NeuralNet::Type t) {
    _edges = edges;
    _netType = t;
    updateNodes();
    replot();
}
