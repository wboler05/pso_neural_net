#ifndef CONFUSIONMATRIXDIAGRAM_H
#define CONFUSIONMATRIXDIAGRAM_H

#include <QWidget>
#include <QLabel>
#include <QVector>
#include <QGridLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
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

    void setLabels(const QStringList & stringList);
    const size_t & numberOfClassifiers() { return _numberOfClassifiers; }

    void updateConfusionMatrix(const ConfusionMatrix & cm);

    size_t cols();  // Widget cols
    size_t rows();  // Widget rows

    TestStatistics getTestStatistics();

private:
    Ui::ConfusionMatrixDiagram *ui;
    size_t _numberOfClassifiers = 0;
    ConfusionMatrix _data;
    std::vector<TestStatistics::TestStruct> _testStats;
    TestStatistics _ts;
    QStringList _classifierLabels;
    ColorTemplate _colorTemplate;

    QPointer<QWidget> _mainWidget;
    QVector<QVector<QPointer<QWidget>>> _table;
    QPointer<QWidget> _accuracyCell;

    void setDefaultColorTemplate();
    void setColorTemplate(ColorTemplate ct);

    void buildMatrix();

    void updateTestStatistics();

    void constructClassLabel(QGridLayout * mainLayout);
    void constructActualPredictLabels(QGridLayout * mainLayout);
    void constructDataTable(QGridLayout * mainLayout);

    QWidget * constructCell(const real & number, const real & ratio, bool truePos);

    QLabel * getNewClassifierLabel(const QString & label);
    QString classifierLabelStyle();

};

#endif // CONFUSIONMATRIXDIAGRAM_H
