#ifndef PARTICLEPLOT_H
#define PARTICLEPLOT_H

#include <QPointF>

#include <qwt_plot.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <qwt_plot_curve.h>

#include <vector>
#include "backend/neuralpso.h"

class ParticlePlot : public QwtPlot
{
public:
    ParticlePlot(QWidget *parent=nullptr);

    void setParticles(const std::shared_ptr<std::vector<NeuralPso::NeuralParticle>> & p) { _particles = p; }
    const std::shared_ptr<std::vector<NeuralPso::NeuralParticle>> & particles() { return _particles; }

public slots:
    void updatePlot(const size_t & firstDim, const size_t & secondDim, const size_t & thirdDim);

private:
    std::shared_ptr<std::vector<NeuralPso::NeuralParticle>> _particles;

    void updatePlot(const size_t &, const real &);
    QwtPlotMarker * getPointMarker(const QPointF & point);
};

#endif // PARTICLEPLOT_H
