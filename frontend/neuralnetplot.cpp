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
    std::vector<std::vector<real>> & enableNodes = (*_state)[0];  //layer : node
    size_t edgeLayers = NeuralNet::totalEdgeLayersFromState(*_state)+1;
    maxX = edgeLayers;

    // Draw nodes and edges (Repeated sentinels are for readability
    for (size_t edgeLayer = 0, stateLayer = 1, nextEdgeLayer = 1;
         edgeLayer < edgeLayers;
         edgeLayer++, stateLayer++, nextEdgeLayer++)
    {
        size_t leftNodes = (*_state)[stateLayer].size();
        if (leftNodes > 0) {
            qreal y_offset = 1.0 / static_cast<double>(leftNodes);
            //maxY = std::max(maxY, (float)_edges->at(i).size());

            for (size_t leftNode = 0; leftNode < leftNodes; leftNode++) {

                qreal x = edgeLayer;
                qreal y = y_offset * static_cast<qreal>(leftNode);

                bool enableNode = !NeuralNet::isSkipNode(*_state, edgeLayer, leftNode);

                QwtPlotMarker * mark = getNodeMarker(QPointF(x, y), enableNode);
                mark->attach(this);

                // Edges
                size_t rightNodes = (*_state)[stateLayer][leftNode].size();
                double next_y_offset = 1.0f / static_cast<double>(rightNodes);
                for (size_t rightNode = 0; rightNode < rightNodes; rightNode++) {
                    enableNode &= !NeuralNet::isSkipNode(*_state, nextEdgeLayer, rightNode);

                    real edgeValue = (*_state)[stateLayer][leftNode][rightNode];
                    qreal xf = nextEdgeLayer;
                    qreal yf = next_y_offset * static_cast<double>(rightNode);

                    QVector<QPointF> edgeData;
                    edgeData.append(QPointF(x, y));
                    edgeData.append(QPointF(xf, yf));

                    QwtPlotCurve * newCurve = new QwtPlotCurve();
                    newCurve->setSamples(edgeData);
                    QColor curveColor = edgeColor(edgeValue, enableNode);
                    double lt = lineThickness * qAbs(edgeValue);
                    //newCurve->setBrush(curveColor);
                    newCurve->setPen(curveColor, lt);
                    newCurve->attach(this);

                }
            }

            // Don't forget to draw output nodes.
            if (edgeLayer == edgeLayers - 1) {
                if ((*_state)[stateLayer].size() > 0) {
                    if ((*_state)[stateLayer][0].size() > 0) {
                        size_t totalOutputNodes = ((*_state)[stateLayer][0].size());
                        y_offset = 1.0f / static_cast<double>(totalOutputNodes);

                        for (size_t outputNode = 0; outputNode < totalOutputNodes; outputNode++) {
                            QwtPlotMarker * mark = getNodeMarker(
                                        QPointF(nextEdgeLayer, y_offset*static_cast<double>(outputNode)),
                                        !NeuralNet::isSkipNode(*_state, nextEdgeLayer, outputNode));
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
                QColor curveColor = edgeColor(edgeVal, true);
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

QColor NeuralNetPlot::edgeColor(const double & val, const bool & enableEdge) {
    if (!enableEdge || val == 0) {
        return QColor(0, 0, 0, 0);
    }

    if (val > 0) {
        return QColor(Qt::blue);
    } else if (val < 0) {
        return QColor(Qt::red);
    }
}

QwtPlotMarker * NeuralNetPlot::getNodeMarker(const QPointF & pos, const bool & enableNode) {
    QColor nodeColor;
    if (enableNode) {
        nodeColor = QColor(Qt::green);
    } else {
        nodeColor = QColor(0, 0, 0, 0);
    }

    QwtSymbol *symbol = new QwtSymbol(QwtSymbol::Ellipse, QBrush(nodeColor), QPen(nodeColor), QSize(5,5));
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
