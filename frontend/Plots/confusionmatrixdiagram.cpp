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

void ConfusionMatrixDiagram::updateConfusionMatrix(const ConfusionMatrix &cm) {

    _numberOfClassifiers = cm.numberOfClassifiers();

    buildMatrix();
}

void ConfusionMatrixDiagram::setLabels(const QStringList & stringList) {
    if (stringList.size() == 0) return;
    _classifierLabels = stringList;
    buildMatrix();
}

void ConfusionMatrixDiagram::buildMatrix() {

    while (QWidget * w = findChild<QWidget *>()) {
        w->deleteLater();
    }

    QGridLayout * mainLayout = new QGridLayout();
    constructActualPredictLabels(mainLayout);
    constructClassLabel(mainLayout);
    constructDataTable(mainLayout);

    if (_mainWidget) {
        layout()->removeWidget(_mainWidget);
    }

    _mainWidget = new QWidget();
    _mainWidget->setLayout(mainLayout);
    layout()->addWidget(_mainWidget);

}

size_t ConfusionMatrixDiagram::rows() {
    return 3 + std::max(_numberOfClassifiers, static_cast<size_t>(1));
}

size_t ConfusionMatrixDiagram::cols() {
    return 3 + std::max(_numberOfClassifiers, static_cast<size_t>(1));
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

    // False Positive
    QLabel * falsePositiveLabel = getNewClassifierLabel("FP");
    mainLayout->addWidget(falsePositiveLabel, 1, _numberOfClassifiers + 2);

    // False Negative
    QLabel * falseNegativeLabel = getNewClassifierLabel("FN");
    mainLayout->addWidget(falseNegativeLabel, _numberOfClassifiers + 2, 1);
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
    _table.resize(_numberOfClassifiers + 1);
    for (int i = 0; i < _table.length(); i++) {
        _table[i].resize(_numberOfClassifiers + 1);
    }

    // Load Table with Data
    for (size_t i = 0; i < _numberOfClassifiers; i++) {
        for (size_t j = 0; j < _numberOfClassifiers; j++) {
//            QPointer<QWidget> newCell = constructCell(_data.getResultValues()[i][j], _data.getResultRatios()[i][j], i == j);
//            _table[i][j] = newCell;
        }
    }

    // Load Fp
    // Load Fn
    // Load Accuracy

}

QWidget * ConfusionMatrixDiagram::constructCell(const real & number, const real & ratio, bool truePos) {
    return nullptr;
}




