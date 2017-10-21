#include "neuralnetplot.h"

#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>

NeuralNetPlot::NeuralNetPlot(QWidget *parent) :
    QwtPlot(parent)
{
    setTitle(QwtText("Neural Net Plot"));

    setAxisAutoScale(yLeft, true);
    setAxisAutoScale(xBottom, true);
}

void NeuralNetPlot::updateNetworkPlot() {
    if (_state==nullptr) return;
    if (_state->size() == 0) return;

    this->detachItems();

//    attachGrid();

    drawNodes();
    drawEdges();

/*
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
*/

}

void NeuralNetPlot::drawNodes() {
    double minX = 0, maxX = 0;
    double minY = 0, maxY = 1;

    size_t totalInnerNodeLayers = NeuralNet::totalInnerNodeLayersFromState(*_state);
    size_t totalNodeLayers = totalInnerNodeLayers + 2;
    if (totalNodeLayers == 0) return;
    maxX = totalNodeLayers-1;

    for (size_t layer = 0; layer < totalNodeLayers-1; layer++) {
        size_t leftLayer = layer+1;
        size_t totalLeftNodes = (*_state)[leftLayer].size();

        qreal y_offset = 1.0;
        if (totalLeftNodes > 0) {
            y_offset = 1.0 / (static_cast<double>(totalLeftNodes));
        }
        for (size_t leftNode = 0; leftNode < totalLeftNodes; leftNode++) {

            qreal x = layer;
            qreal y = y_offset * (static_cast<qreal>(leftNode) + 0.5);

            bool enableNode = !NeuralNet::isSkipNode(*_state, layer-1, leftNode);

            QwtPlotMarker * mark = getNodeMarker(QPointF(x, y), enableNode);
            mark->attach(this);

        }
        if (layer == totalNodeLayers - 2) {
            size_t totalOutputNodes = (*_state)[leftLayer][0].size();
            y_offset = 1.0 / (static_cast<double>(totalOutputNodes));
            for (size_t outputNode = 0; outputNode < totalOutputNodes; outputNode++) {
                qreal x = layer+1;
                qreal y = y_offset * (static_cast<qreal>(outputNode) + 0.5);

                bool enableNode = !NeuralNet::isSkipNode(*_state, layer, outputNode);

                QwtPlotMarker * mark = getNodeMarker(QPointF(x, y), enableNode);
                mark->attach(this);
            }
        }
    }

    setAxisScale(xBottom, minX-0.25, maxX+0.25);
    setAxisScale(yLeft, minY-0.05, maxY+0.05);
}

void NeuralNetPlot::drawEdges() {
    const double lineThickness = 5;
    size_t totalEdges = NeuralNet::totalEdgeLayersFromState(*_state);

    for (size_t edge = 0; edge < totalEdges; edge++) {
        size_t stateEdgeIt = edge + 1;
        size_t totalLeftNodes = (*_state)[stateEdgeIt].size();
        double y_offset = 1.0 / (static_cast<double>(totalLeftNodes));

        for (size_t leftNode = 0; leftNode < totalLeftNodes; leftNode++) {
            size_t totalRightNodes = (*_state)[stateEdgeIt][leftNode].size();

            qreal x = edge;
            qreal y = y_offset * (static_cast<qreal>(leftNode) + 0.5);
            bool enableLeftNode = !NeuralNet::isSkipNode(*_state, edge-1, leftNode);

            int nodeOffest = 1;
            if (edge == totalEdges-1) {
                nodeOffest = 0;
            }
            double next_y_offset = 1.0 / (static_cast<double>(totalRightNodes+nodeOffest));
            for (size_t rightNode = 0; rightNode < totalRightNodes; rightNode++) {

                bool nextEnableNode = enableLeftNode &&
                        !NeuralNet::isSkipNode(*_state, edge, rightNode);

                real edgeValue = (*_state)[stateEdgeIt][leftNode][rightNode];
                qreal xf = edge+1;
                qreal yf = next_y_offset * (static_cast<qreal>(rightNode) + 0.5);

                QVector<QPointF> edgeData;
                edgeData.append(QPointF(x, y));
                edgeData.append(QPointF(xf, yf));

                QwtPlotCurve * newCurve = new QwtPlotCurve();
                newCurve->setSamples(edgeData);
                QColor curveColor = edgeColor(edgeValue, nextEnableNode);
                double lt = lineThickness * qAbs(edgeValue);
                newCurve->setPen(curveColor, lt);
                newCurve->attach(this);

            }
        }
    }
}

void NeuralNetPlot::drawRecurrentEdges() {

}

void NeuralNetPlot::attachGrid() {
    QwtPlotGrid * grid = new QwtPlotGrid();
    grid->enableX(true);
    grid->enableY(true);
    grid->attach(this);
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
    QColor outlineColor;
    if (enableNode) {
        nodeColor = QColor(Qt::green);
        outlineColor = QColor(Qt::green);
    } else {
        nodeColor = QColor(0, 0, 0, 0);
        outlineColor = QColor(Qt::black);
    }

    QwtSymbol *symbol = new QwtSymbol(QwtSymbol::Ellipse, QBrush(nodeColor),
                                QPen(outlineColor), QSize(5,5));
    QwtPlotMarker *mark = new QwtPlotMarker();
    mark->setSymbol(symbol);
    mark->setValue(pos);
    return mark;
}

void NeuralNetPlot::setState(NeuralNet::State * state, NeuralNet::Type t) {
    _state = state;
    _netType = t;
    updateNetworkPlot();
    replot();
}
