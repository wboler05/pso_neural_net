#include "confusionmatrixdiagram.h"
#include "ui_confusionmatrixdiagram.h"

ConfusionMatrixDiagram::ConfusionMatrixDiagram(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfusionMatrixDiagram)
{
    ui->setupUi(this);
}

ConfusionMatrixDiagram::~ConfusionMatrixDiagram()
{
    delete ui;
}

void ConfusionMatrixDiagram::updateConfusionMatrix(const ClassifierCounts & cl) {
    if (!validateNewResults(cl)) return;

    setNumberOfClassifiers(cl.size());
    calculateTableValues(cl);
    buildMatrix();
}

bool ConfusionMatrixDiagram::validateNewResults(const ClassifierCounts & cl) {
    size_t N = cl.size();
    for (size_t i = 0; i < N; i++) {
        if (cl[i].size() != N) return false;
    }
    return true;
}

void ConfusionMatrixDiagram::setLabels(const QStringList & stringList) {
    if (stringList.size() == 0) return;
    setNumberOfClassifiers(static_cast<size_t>(stringList.size()));
}

void ConfusionMatrixDiagram::setNumberOfClassifiers(const size_t & n) {
    _numberOfClassifiers = n;
    _results.resize(n);
    _resultRatios.resize(n);
    for (size_t i = 0; i < n; i++) {
        _results[i].resize(n, 0);
        _resultRatios[i].resize(n, 0);
    }
}

void ConfusionMatrixDiagram::calculateTableValues(const ClassifierCounts & cl) {
    size_t maxVal = 0;
    for (size_t i = 0; i < _results.size(); i++) {
        for (size_t j = 0; j < _results[i].size(); j++) {
            maxVal = std::max(maxVal, _results[i][j]);
        }
    }

    real realMaxVal = static_cast<real>(maxVal);

    for (size_t i = 0; i < _results.size(); i++) {
        for (size_t j = 0; j < _results[i].size(); j++) {
            real val = static_cast<real>(_results[i][j]) / realMaxVal;
            _resultRatios[i][j] = val;
        }
    }

    for (size_t i = 0; i < _results.size(); i++) {
        for (size_t j = 0; j < _results[i].size(); j++) {
            if (i == j) {
                a;sldkjf;laskdjf
                        // Wait, we need to get the actuals as well. We only have results
            }
        }
    }
}

void ConfusionMatrixDiagram::buildMatrix() {
    if (_mainWidget) {
        _mainWidget->deleteLater();
    }

    size_t rows_ = rows();
    size_t cols_ = cols();

    QGridLayout * mainLayout = new QGridLayout();
    constructActualPredictLabels(mainLayout);
    constructClassLabel(mainLayout);
    constructDataTable(mainLayout);
}

size_t ConfusionMatrixDiagram::rows() {
    return 4 + std::max(_numberOfClassifiers, static_cast<size_t>(1));
}

size_t ConfusionMatrixDiagram::cols() {
    return 4 + std::max(_numberOfClassifiers, static_cast<size_t>(1));
}

void ConfusionMatrixDiagram::constructActualPredictLabels(QGridLayout * mainLayout) {
    QLabel * horzLabel = new QLabel("Actual");
    QGraphicsScene * horzScene = new QGraphicsScene();
    QGraphicsProxyWidget * horzProxy = horzScene->addWidget(horzLabel);
    QGraphicsView * horzView = new QGraphicsView(horzScene);
    mainLayout->addWidget(horzView, 0, 1, 1, cols()-1);

    QLabel * vertLabel = new QLabel("Predicted");
    QGraphicsScene * vertScene = new QGraphicsScene();
    QGraphicsProxyWidget * vertProxy = vertScene->addWidget(vertLabel);
    vertProxy->setRotation(-90);
    QGraphicsView * vertView = new QGraphicsView(vertScene);
    mainLayout->addWidget(vertView, 1, 0, rows()-1, 1);
}

void ConfusionMatrixDiagram::constructClassLabel(QGridLayout * mainLayout) {
    for (int i = 0; i < static_cast<int>(_numberOfClassifiers); i++){
        int offsetIndex = i+2;
        QString columnLabel = _classifierLabels[i];

        QLabel * columnLabelWidget = getNewClassifierLabel(columnLabel);
        mainLayout->addWidget(columnLabelWidget, 1, offsetIndex);

        QLabel * rowLabelWidget = getNewClassifierLabel(columnLabel);
        mainLayout->addWidget(rowLabelWidget, offsetIndex, 1);
    }

    // True Positive
    QLabel * truePositiveLabel = getNewClassifierLabel("TP");
    mainLayout->addWidget(truePositiveLabel, 1, _numberOfClassifiers + 2);

    // False Positive
    QLabel * falsePositiveLabel = getNewClassifierLabel("FP");
    mainLayout->addWidget(falsePositiveLabel, 1, _numberOfClassifiers + 3);

    // True Negative
    QLabel * trueNegativeLabel = getNewClassifierLabel("TN");
    mainLayout->addWidget(trueNegativeLabel, _numberOfClassifiers + 2, 1);

    // False Negative
    QLabel * falseNegativeLabel = getNewClassifierLabel("FN");
    mainLayout->addWidget(falseNegativeLabel, _numberOfClassifiers + 3, 1);
}

QLabel * ConfusionMatrixDiagram::getNewClassifierLabel(const QString & label) {
    QLabel * newLabel = new QLabel(label);
    newLabel->setStyleSheet(classifierLabelStyle());
    return newLabel;
}

QString ConfusionMatrixDiagram::classifierLabelStyle() {
    QString stylesheet;
    stylesheet.append("{ background-color: ");
    stylesheet.append(_colorTemplate.classifierLabels.background().color().toRgb().name());
    stylesheet.append(" }");
    return stylesheet;
}

void ConfusionMatrixDiagram::constructDataTable(QGridLayout * mainLayout) {
    // Initialize Table
    _table.clear();
    _table.resize(_numberOfClassifiers + 2);
    for (int i = 0; i < _table.length(); i++) {
        _table[i].resize(_numberOfClassifiers + 2);
    }

    // Load Table with Data
    for (size_t i = 0; i < _numberOfClassifiers; i++) {
        for (size_t j = 0; j < _numberOfClassifiers; j++) {
            QPointer<QWidget> newCell = constructCell(_results[i][j], _resultRatios[i][j], i == j);
            _table[i][j] = newCell;
        }
    }



    for (int i = 0; i < _numberOfClassifiers; i++) {
        for (int j = 0; j < _numberOfClassifiers; j++) {

        }
    }
}




