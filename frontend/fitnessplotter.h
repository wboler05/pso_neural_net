#ifndef FITNESSPLOTTER_H
#define FITNESSPLOTTER_H

#include <qwt_plot.h>
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

    void plotHistory(const std::vector<real> & history);
    void setLineColor(const QColor & c);
    void setMarkerColor(const QColor & c);

private:
    Ui::FitnessPlotter *ui;

    QColor _plotLineColor;
    QColor _plotMarkColor;
};

#endif // FITNESSPLOTTER_H
