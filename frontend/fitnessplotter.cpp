#include "fitnessplotter.h"
#include "ui_fitnessplotter.h"

FitnessPlotter::FitnessPlotter(QWidget *parent) :
    QwtPlot(parent),
    ui(new Ui::FitnessPlotter),
    _plotLineColor(QColor(Qt::red)),
    _plotMarkColor(QColor(Qt::red))
{
    ui->setupUi(this);

    this->setTitle("Fitness Plot");
    setAxisTitle(QwtPlot::xBottom, QString("Epochs"));
    setAxisTitle(QwtPlot::yLeft, QString("Fitness"));
}

FitnessPlotter::~FitnessPlotter()
{
    delete ui;
}

void FitnessPlotter::setLineColor(const QColor & c) {
    _plotLineColor = c;
}

void FitnessPlotter::setMarkerColor(const QColor & c) {
    _plotMarkColor = c;
}

void FitnessPlotter::plotHistory(const std::vector<real> &history) {

    if (history.size() == 0) return;

    static bool entry = false;

    if (entry) {
        return;
    } else {
        entry = true;
    }

    detachItems();

    QVector<QPointF> plotPoints;
    double lt = 2;

    float minY = std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();

    int window_min = static_cast<int>(history.size()) - static_cast<int>(_windowSize);
    window_min = std::max(window_min, 0);

    for (size_t i = window_min; i < history.size(); i++) {
        QPointF newPoint;
        newPoint.setX((qreal) i+1);
        newPoint.setY((qreal) history[i]);

        minY = std::min(minY, (float)history[i]);
        maxY = std::max(maxY, (float)history[i]);

        plotPoints.append(newPoint);
    }

    QwtPlotCurve * curve = new QwtPlotCurve();
    curve->setPen(_plotLineColor, lt);
    curve->setSamples(plotPoints);
    curve->attach(this);

    for (size_t i = 0; i < plotPoints.length(); i++) {

        QwtSymbol * symbol = new QwtSymbol(
                    QwtSymbol::Ellipse,
                    QBrush(_plotMarkColor),
                    QPen(_plotMarkColor),
                    QSize(5,5));
        QwtPlotMarker * mark = new QwtPlotMarker();
        mark->setSymbol(symbol);
        mark->setValue(plotPoints.at(i));
        mark->attach(this);
    }

    setAxisScale(xBottom, window_min, history.size());
    setAxisScale(yLeft, minY * 0.9995 -0.0025, maxY * 1.0005 + 0.0025);

    replot();

    entry = false;
}


