#ifndef FITNESSPLOTTER_H
#define FITNESSPLOTTER_H

#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <QColor>
#include "PSO/pso.h"

namespace Ui {
class FitnessPlotter;
}

class FitnessPlotter : public QwtPlot
{

public:
    explicit FitnessPlotter(QWidget *parent = 0);
    ~FitnessPlotter();

    void setWindowSize(const size_t & ws) { _windowSize = ws; }
    const size_t & windowSize() { return _windowSize; }

    void plotHistory(const std::vector<real> & history);
    void setLineColor(const QColor & c);
    void setMarkerColor(const QColor & c);

private:
    Ui::FitnessPlotter *ui;
    size_t _windowSize=100;

    QColor _plotLineColor;
    QColor _plotMarkColor;

    void attachGrid();
};

#endif // FITNESSPLOTTER_H
