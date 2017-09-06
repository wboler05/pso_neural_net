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

    detachItems();

    QVector<QPointF> plotPoints;
    double lt = 2;

    float minY = std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();

    for (size_t i = 0; i < history.size(); i++) {
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

    setAxisScale(xBottom, 0, history.size());
    setAxisScale(yLeft, minY * 0.9995, maxY * 1.0005);

    replot();
}


