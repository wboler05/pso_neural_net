#include "particleplot.h"

ParticlePlot::ParticlePlot(QWidget * parent) : QwtPlot(parent)
{

}

void ParticlePlot::updatePlot(const size_t & firstDim, const size_t & secondDim, const size_t & thirdDim) {
    if (_particles == nullptr) {
        return;
    }

    detachItems();

    for (size_t i = 0; i < _particles->size(); i++) {
        NeuralPso::NeuralParticle & p = (*_particles)[i];
        if (firstDim < p._x.size()) {
            if (secondDim < p._x[firstDim].size()) {
                if (thirdDim < p._x[firstDim][secondDim].size()) {
                    updatePlot(i, p._x[firstDim][secondDim][thirdDim]);
                }
            }
        }

    }
    replot();
}

void ParticlePlot::updatePlot(const size_t & particleId, const real & value) {
    QPointF point(static_cast<qreal>(particleId), static_cast<qreal>(value));
    QwtPlotMarker * mark = getPointMarker(point);
    mark->attach(this);

    QVector<QPointF> lineData;
    lineData.append(QPointF(point.x(), 0));
    lineData.append(point);
    QwtPlotCurve * newCurve = new QwtPlotCurve();
    newCurve->setSamples(lineData);
    QColor lineColor(Qt::cyan);
    newCurve->setPen(lineColor, 2);
    newCurve->attach(this);
}

QwtPlotMarker * ParticlePlot::getPointMarker(const QPointF & point) {
    QwtSymbol * symbol = new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::black), QPen(Qt::cyan), QSize(2,2));
    QwtPlotMarker * mark = new QwtPlotMarker();
    mark->setSymbol(symbol);
    mark->setValue(point);
    return mark;
}
