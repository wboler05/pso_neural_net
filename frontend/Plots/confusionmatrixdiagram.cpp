#include "confusionmatrixdiagram.h"
#include "ui_confusionmatrixdiagram.h"

ConfusionMatrixDiagram::ConfusionMatrixDiagram(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfusionMatrixDiagram)
{
    ui->setupUi(this);
    initializeColorTemplates();
}

ConfusionMatrixDiagram::~ConfusionMatrixDiagram()
{
    delete ui;
}

void ConfusionMatrixDiagram::initializeColorTemplates() {

    // Background Colors
    _colorTemplate.background.setColor(QPalette::Background, QColor(Qt::white));
    _colorTemplate.background.setColor(QPalette::Foreground, QColor(Qt::black));

    // Classifier Label Colors
    _colorTemplate.classifierLabels.setColor(QPalette::Background, QColor(239, 242, 215));
    _colorTemplate.classifierLabels.setColor(QPalette::Foreground, QColor(49, 51, 39));

    // Near Zero Colors
    _colorTemplate.nearZero.setColor(QPalette::Background, QColor(237, 248, 249));
    _colorTemplate.nearZero.setColor(QPalette::Foreground, QColor(Qt::black));

    // Near Max Colors
    _colorTemplate.nearMax.setColor(QPalette::Background, QColor(25, 206, 234));
    _colorTemplate.nearMax.setColor(QPalette::Foreground, QColor(Qt::black));


}

void ConfusionMatrixDiagram::updateConfusionMatrix(const ConfusionMatrix &cm) {

    _numberOfClassifiers = cm.numberOfClassifiers();

    buildMatrix();
}

/**
 * @brief ConfusionMatrixDiagram::setLabels
 * @details Must be defined by user.
 * @param stringList
 */
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
            QPointer<QWidget> newCell = constructDataCell(_data.getResultValues()[i][j], _data.getResultRatios()[i][j]);
            _table[i][j] = newCell;
        }
    }

    // Load Fp
    for (size_t i = 0; i < _numberOfClassifiers; i++) {
        QPointer<QWidget> newCell = constructFPNCell(_data.getFalsePositiveValues()[i], _data.getFalsePositiveRatios()[i]);
        _table[i][_numberOfClassifiers-1] = newCell;
    }

    // Load Fn
    for (size_t i = 0; i < _numberOfClassifiers; i++) {
        QPointer<QWidget> newCell = constructFPNCell(_data.getFalseNegativeValues()[i], _data.getFalseNegativeRatios()[i]);
        _table[_numberOfClassifiers-1][i] = newCell;
    }

    // Load Accuracy
    QPointer<QWidget> newCell = constructDataCell(CustomMath::total(_data.getTruePositiveValues()), _data.overallError().accuracy);
    _table[_numberOfClassifiers - 1][_numberOfClassifiers - 1] = newCell;

    _built = true;
}

/**
 * @brief ConfusionMatrixDiagram::constructDataCell
 * @param number
 * @param ratio
 * @return
 */
QWidget * ConfusionMatrixDiagram::constructDataCell(const real & number, const real & ratio) {
    QColor background = linearGradient(ratio, _colorTemplate.nearZero.background().color(), _colorTemplate.nearMax.background().color());
    QColor foreground = linearGradient(ratio, _colorTemplate.nearZero.foreground().color(), _colorTemplate.nearMax.foreground().color());
    return constructCell(number, ratio, background, foreground);
}

/**
 * @brief ConfusionMatrixDiagram::constructFPNCell
 * @param number
 * @param ratio
 * @return
 */
QWidget * ConfusionMatrixDiagram::constructFPNCell(const real & number, const real & ratio) {
    QColor background = linearGradient(ratio, _colorTemplate.nearZero.background().color(), _colorTemplate.nearMax.background().color());
    QColor foreground = linearGradient(ratio, _colorTemplate.nearZero.foreground().color(), _colorTemplate.nearMax.foreground().color());
    return constructCell(number, ratio, background, foreground);
}

/**
 * @brief ConfusionMatrixDiagram::constructCell
 * @param number
 * @param ratio
 * @param background
 * @param foreground
 * @return
 */
QWidget * ConfusionMatrixDiagram::constructCell(const real & number, const real & ratio, const QColor & background, const QColor & foreground) {
    QPointer<QWidget> newCell = new QWidget();
    QVBoxLayout * layout = new QVBoxLayout();
    QLabel * val_lbl = new QLabel();
    val_lbl->setText(QString::number(number));
    layout->addWidget(val_lbl);
    QLabel * ratio_lbl = new QLabel();
    ratio_lbl->setText(QString::number(ratio));
    layout->addWidget(ratio_lbl);
    newCell->setLayout(layout);

    QString lbl_styleSheet;
    lbl_styleSheet.append("color: ");
    //lbl_styleSheet.append(_colorTemplate.nearZero.foreground().color().name());
    lbl_styleSheet.append(foreground.name());
    val_lbl->setStyleSheet(lbl_styleSheet);
    ratio_lbl->setStyleSheet(lbl_styleSheet);

    QString styleSheet;
    styleSheet.append("background-color: ");
    //styleSheet.append(_colorTemplate.nearZero.background().color().name());
    styleSheet.append(background.name());
    newCell->setStyleSheet(styleSheet);

    return newCell;
}

QColor ConfusionMatrixDiagram::linearGradient(const real & val, const real & minVal, const real & maxVal, const QColor & lowColor, const QColor & highColor) {
    int r = static_cast<int>(linearGradientChannel(val, minVal, maxVal, lowColor.redF(), highColor.redF()) * 255.0);
    int g = static_cast<int>(linearGradientChannel(val, minVal, maxVal, lowColor.greenF(), highColor.greenF()) * 255.0);
    int b = static_cast<int>(linearGradientChannel(val, minVal, maxVal, lowColor.blueF(), highColor.blueF()) * 255.0);
    int a = static_cast<int>(linearGradientChannel(val, minVal, maxVal, lowColor.alphaF(), highColor.alphaF()) * 255.0);

    return QColor(r, g, b, a);
}

double ConfusionMatrixDiagram::linearGradientChannel(const real & val, const real & minVal, const real & maxVal, const double & lowColor, const double & highColor) {
    double numerator = highColor * (val - minVal) + lowColor * (maxVal - val);
    return numerator / (maxVal - minVal);
}

QColor ConfusionMatrixDiagram::linearGradient(const real & ratio, const QColor & lowColor, const QColor & highColor) {
    int r = static_cast<int>(linearGradientChannel(ratio, lowColor.redF(), highColor.redF()) * 255.0);
    int g = static_cast<int>(linearGradientChannel(ratio, lowColor.greenF(), highColor.greenF()) * 255.0);
    int b = static_cast<int>(linearGradientChannel(ratio, lowColor.blueF(), highColor.blueF()) * 255.0);
    int a = static_cast<int>(linearGradientChannel(ratio, lowColor.alphaF(), highColor.alphaF()) * 255.0);
    return QColor(r, g, b, a);
}

double ConfusionMatrixDiagram::linearGradientChannel(const real & ratio, const double & lowColor, const double & highColor) {
    real m_ratio = ratio;
    if (m_ratio < 0) m_ratio = 0;
    if (m_ratio > 1) m_ratio = 1;

    return ( m_ratio * (highColor - lowColor) ) + lowColor;
}




