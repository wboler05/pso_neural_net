#ifndef CONFUSIONMATRIXDIAGRAM_H
#define CONFUSIONMATRIXDIAGRAM_H

#include <QWidget>
#include <QLabel>
#include <QVector>
#include <QGridLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTableWidget>
#include <QGraphicsProxyWidget>

#include "backend/confusionmatrix.h"

namespace Ui {
class ConfusionMatrixDiagram;
}

class ConfusionMatrixDiagram : public QWidget
{
    Q_OBJECT

public:

    struct ColorTemplate {
        QPalette background;
        QPalette classifierLabels;
        QPalette nearZero;
        QPalette nearMax;
    };

    explicit ConfusionMatrixDiagram(QWidget *parent = 0);
    ~ConfusionMatrixDiagram();

    void initializeColorTemplates();

    void setLabels(const QStringList & stringList);
    void setNumberOfClassifiers(const size_t & n) { _numberOfClassifiers = n; }
    const size_t & numberOfClassifiers() { return _numberOfClassifiers; }

    void updateConfusionMatrix(const ConfusionMatrix & cm);

    size_t cols();  // Widget cols
    size_t rows();  // Widget rows

    TestStatistics getTestStatistics();

    static QColor linearGradient(const real & val, const real & minVal, const real & maxVal, const QColor & lowColor, const QColor & highColor);
    static double linearGradientChannel(const real & val, const real & minVal, const real & maxVal, const double & lowColor, const double & highColor);

    static QColor linearGradient(const real & ratio, const QColor & lowColor, const QColor & highColor);
    static double linearGradientChannel(const real & ratio, const double & lowColor, const double & highColor);

    bool built () { return _built; }
    void updateMatrix();

protected slots:
    void updateFromCM();

private:
    Ui::ConfusionMatrixDiagram *ui;
    size_t _numberOfClassifiers = 0;
    ConfusionMatrix _data;
    std::vector<TestStatistics::TestStruct> _testStats;
    TestStatistics _ts;
    QStringList _classifierLabels;
    ColorTemplate _colorTemplate;
    bool _built = false;

    QGraphicsView * _horzView = nullptr;
    QGraphicsView * _vertView = nullptr;

    //QPointer<QWidget> _mainWidget;
    QTableWidget _tableWidget;
    //QVector<QVector<QPointer<QWidget>>> _table;
    //QPointer<QWidget> _accuracyCell;

    void setDefaultColorTemplate();
    void setColorTemplate(ColorTemplate ct);

    void buildMatrix();

    void updateTestStatistics();

    void constructClassLabel();
    void constructActualPredictLabels();
    void constructDataTable();

    QWidget * constructDataCell(const real & number, const real & ratio);
    QWidget * constructFPNCell(const real & number, const real & ratio);
    QWidget * constructCell(const real & number, const real & ratio, const QColor & background, const QColor & foreground);

    QLabel * getNewClassifierLabel(const QString & label);
    QString classifierLabelStyle();
    QString styleSheetColorMaker(const QString & label, const QColor & color);
    QString styleSheetColorMaker(const QVector<QString> & labels, const QVector<QColor> & colors);
    QString colorToStyleSheet(const QColor & color);

};

#endif // CONFUSIONMATRIXDIAGRAM_H
