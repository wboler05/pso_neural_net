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
    if (_state==nullptr) return;

    this->detachItems();

    const double lineThickness = 5;

    double minX = 0, maxX = 0;
    double minY = 0, maxY = 1;

    // Plot Nodes
    size_t edgeLayers = NeuralNet::totalEdgeLayersFromState(*_state)+1;
    maxX = edgeLayers;

    // Draw nodes and edges
    for (size_t i = 0; i < edgeLayers; i++) {
        size_t leftNodes = (*_state)[i+1].size();
        if (leftNodes > 0) {
            qreal y_offset = 1.0 / static_cast<double>(leftNodes);
            //maxY = std::max(maxY, (float)_edges->at(i).size());

            for (size_t j = 0; j < leftNodes; j++) {

                qreal x = i;
                qreal y = y_offset * static_cast<qreal>(j);

                QwtPlotMarker * mark = getNodeMarker(QPointF(x, y));
                mark->attach(this);

                // Edges
                size_t rightNodes = (*_state)[i+1][j].size();
                double next_y_offset = 1.0f / static_cast<double>(rightNodes);
                for (size_t k = 0; k < rightNodes; k++) {
                    real edgeValue = (*_state)[i+1][j][k];
                    qreal xf = i+1;
                    qreal yf = next_y_offset * (double) k;

                    QVector<QPointF> edgeData;
                    edgeData.append(QPointF(x, y));
                    edgeData.append(QPointF(xf, yf));

                    QwtPlotCurve * newCurve = new QwtPlotCurve();
                    newCurve->setSamples(edgeData);
                    QColor curveColor = edgeColor(edgeValue);
                    double lt = lineThickness * qAbs(edgeValue);
                    //newCurve->setBrush(curveColor);
                    newCurve->setPen(curveColor, lt);
                    newCurve->attach(this);

                }
            }

            // Don't forget to draw output nodes.
            if (i == edgeLayers - 1) {
                size_t it = i+1;
                if ((*_state)[it].size() > 0) {
                    if ((*_state)[it][0].size() > 0) {
                        size_t totalOutputNodes = ((*_state)[it][0].size());
                        y_offset = 1.0f / static_cast<double>(totalOutputNodes);

                        for (size_t j = 0; j < totalOutputNodes; j++) {
                            QwtPlotMarker * mark = getNodeMarker(QPointF(it, y_offset*(double)j));
                            mark->attach(this);
                        }
                    }
                }
            }
        }
    }

    // Draw Recurrent Loops
    if (_netType == NeuralNet::Recurrent) {
        size_t beginIt = edgeLayers + 1;

        const qreal rx = 0.05;
        const qreal ry = 0.03;
        const qreal rad_res = 360;
        qreal radStep = 360.0 / (2.0 * M_PI * rad_res);
        qreal maxArc = (2.0 * M_PI) * ( 300.0 / 360.0 );

        for (size_t i = 0; i < edgeLayers-2; i++) {
            size_t totalRecNodes = (*_state)[beginIt + i].size();
            double y_offset = 1.0 / static_cast<double>(totalRecNodes);
            for (size_t j = 0; j < totalRecNodes; j++) {
                qreal x = i+1;
                qreal y = y_offset * static_cast<double>(j);
                size_t lastIt = (*_state)[beginIt + i][j].size();
                real edgeVal = (*_state)[beginIt + i][j][lastIt];

                QVector<QPointF> edgeData;
                QwtPlotCurve * newCurve = new QwtPlotCurve();
                QColor curveColor = edgeColor(edgeVal);
                double lt = lineThickness * qAbs(edgeVal);
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

void NeuralNetPlot::setState(NeuralNet::State * state, NeuralNet::Type t) {
    _state = state;
    _netType = t;
    updateNodes();
    replot();
}
