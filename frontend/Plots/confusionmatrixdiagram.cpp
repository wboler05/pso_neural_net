#include "confusionmatrixdiagram.h"
#include "ui_confusionmatrixdiagram.h"

#include <QTimer>

ConfusionMatrixDiagram::ConfusionMatrixDiagram(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfusionMatrixDiagram)
{
    ui->setupUi(this);
    initializeColorTemplates();

    QTimer * updateTimer = new QTimer();
    updateTimer->setInterval(67);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateFromCM()));
    updateTimer->start();

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
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
    if (cm.numberOfClassifiers() == 0 && _numberOfClassifiers == 0) return;
    else if (cm.numberOfClassifiers() == 0) _tableWidget.clearContents();

    if (cm.numberOfClassifiers() != _numberOfClassifiers) {
        _numberOfClassifiers = cm.numberOfClassifiers();
        _built = false;
    }

    _data = cm;
}

void ConfusionMatrixDiagram::updateFromCM() {
    if (_data.numberOfClassifiers() == 0) return;

    if (!_built) {
        buildMatrix();
    } else {
        updateMatrix();
    }
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
    _tableWidget.clear();

    constructActualPredictLabels();
    constructDataTable();

    constructLayout();

    QHeaderView::ResizeMode cellResizeMode = QHeaderView::ResizeToContents;
    //QHeaderView::ResizeMode cellResizeMode = QHeaderView::Stretch;

    _tableWidget.horizontalHeader()->setSectionResizeMode(cellResizeMode);
    _tableWidget.verticalHeader()->setSectionResizeMode(cellResizeMode);

    show();
}

void ConfusionMatrixDiagram::initializeClassifierLabels() {
    _classifierLabels.clear();
    for (size_t i = 0; i < numberOfClassifiers(); i++) {
        QString s;
        s.append('C');
        s.append(QString::number(i));
        _classifierLabels.append(s);
    }
}

void ConfusionMatrixDiagram::constructLayout() {
    QGridLayout * glayout = new QGridLayout(this);
    glayout->addWidget(_vertView, 1, 0);
    glayout->addWidget(_horzView, 0, 1);
    glayout->addWidget(&_tableWidget, 1, 1);
    glayout->setColumnStretch(0, 0);
    glayout->setColumnStretch(1, 1000);
    glayout->setRowStretch(0, 0);
    glayout->setRowStretch(1, 1000);

    glayout->setColumnMinimumWidth(0, 5);
    glayout->setColumnMinimumWidth(1, 245);
    glayout->setRowMinimumHeight(0, 5);
    glayout->setRowMinimumHeight(1, 245);
    setLayout(glayout);
}

void ConfusionMatrixDiagram::updateMatrix() {
    updateDataTable();
    //adjustSize();
    update();
}

size_t ConfusionMatrixDiagram::rows() {
    return 3 + std::max(_numberOfClassifiers, static_cast<size_t>(1));
}

size_t ConfusionMatrixDiagram::cols() {
    return 3 + std::max(_numberOfClassifiers, static_cast<size_t>(1));
}

void ConfusionMatrixDiagram::constructActualPredictLabels() {
    QLabel * horzLabel = new QLabel("Actual");
    horzLabel->setMinimumHeight(12);
    QGraphicsScene * horzScene = new QGraphicsScene();
    QGraphicsProxyWidget * horzProxy = horzScene->addWidget(horzLabel);
    _horzView = new QGraphicsView(horzScene);
    _horzView->setMinimumSize(20, 20);
    _horzView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QLabel * vertLabel = new QLabel("Predicted");
    vertLabel->setMinimumHeight(12);
    QGraphicsScene * vertScene = new QGraphicsScene();
    QGraphicsProxyWidget * vertProxy = vertScene->addWidget(vertLabel);
    vertProxy->setMinimumHeight(12);
    vertProxy->setMinimumWidth(12);
    vertProxy->setRotation(-90);
    _vertView = new QGraphicsView(vertScene);
    _vertView->setMinimumSize(20, 20);
    _vertView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void ConfusionMatrixDiagram::constructClassLabel() {
    initializeClassifierLabels();

    QStringList horzLabels = _classifierLabels;
    horzLabels.append("FN");
    _tableWidget.setHorizontalHeaderLabels(horzLabels);

    QStringList vertLabels = _classifierLabels;
    vertLabels.append("FP");
    _tableWidget.setVerticalHeaderLabels(vertLabels);
}

QLabel * ConfusionMatrixDiagram::getNewClassifierLabel(const QString & label) {
    QLabel * newLabel = new QLabel(label);
//    newLabel->setStyleSheet(classifierLabelStyle()); // NOT ALLOWED ON QLabel
    return newLabel;
}

QString ConfusionMatrixDiagram::classifierLabelStyle() {
    return styleSheetColorMaker(QString("background-color"), _colorTemplate.classifierLabels.background().color());
}

QString ConfusionMatrixDiagram::styleSheetColorMaker(const QString & label, const QColor & color) {
    QString stylesheet;
    //stylesheet.append("{ ");
    stylesheet.append(label);
    stylesheet.append(colorToStyleSheet(color));
    //stylesheet.append("}");
    return stylesheet;
}

QString ConfusionMatrixDiagram::colorToStyleSheet(const QColor & color) {
    QString stylesheet;
    stylesheet.append(QString(": rgba(%1,%2,%3,%4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha()));
    return stylesheet;
}

QString ConfusionMatrixDiagram::styleSheetColorMaker(const QVector<QString> & labels, const QVector<QColor> & colors) {
    if (labels.length() != colors.length()) return QString();
    if (labels.length() == 0) return QString();

    QString stylesheet;
    stylesheet.append("{ ");
    for (int i = 0; i < labels.length(); i++) {
        stylesheet.append(labels[i]);
        stylesheet.append(colorToStyleSheet(colors[i]));
        if (i != labels.length() -1) {
            stylesheet.append(',');
        }
    }
    stylesheet.append('}');
    return stylesheet;
}

void ConfusionMatrixDiagram::constructDataTable() {
    // Initialize Table
    _tableWidget.clearContents();
    _tableWidget.setColumnCount(_numberOfClassifiers+1);
    _tableWidget.setRowCount(_numberOfClassifiers+1);

    updateDataTable();

    _built = true;
}

void ConfusionMatrixDiagram::updateDataTable() {
    constructClassLabel();

    // Load Table with Data
    for (size_t i = 0; i < _numberOfClassifiers; i++) {
        for (size_t j = 0; j < _numberOfClassifiers; j++) {
            QPointer<QWidget> newCell = constructDataCell(_data.getResultValues()[i][j], _data.getResultRatios()[i][j]);
            _tableWidget.setCellWidget(j, i, newCell);
            //_table[i][j] = newCell;
        }
    }

    // Load Fp
    for (size_t i = 0; i < _numberOfClassifiers; i++) {
        QPointer<QWidget> newCell = constructFPNCell(_data.getFalsePositiveValues()[i], _data.getFalsePositiveRatios()[i]);
        _tableWidget.setCellWidget(_numberOfClassifiers, i, newCell);
        //_table[i][_numberOfClassifiers] = newCell;
    }

    // Load Fn
    for (size_t i = 0; i < _numberOfClassifiers; i++) {
        QPointer<QWidget> newCell = constructFPNCell(_data.getFalseNegativeValues()[i], _data.getFalseNegativeRatios()[i]);
        _tableWidget.setCellWidget(i, _numberOfClassifiers, newCell);
        //_table[_numberOfClassifiers][i] = newCell;
    }

    // Load Accuracy
    if (_numberOfClassifiers > 0) {
        QPointer<QWidget> newCell = constructDataCell(CustomMath::total(_data.getTruePositiveValues()), _data.overallError().accuracy);
        _tableWidget.setCellWidget(_numberOfClassifiers, _numberOfClassifiers, newCell);
        //_table[_numberOfClassifiers][_numberOfClassifiers] = newCell;
    }
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
    QVBoxLayout * layout = new QVBoxLayout(newCell);
    layout->setMargin(0);
    layout->setSpacing(2);

    QLabel * val_lbl = constructDataCellLabel(number, 0);
    layout->addWidget(val_lbl);
    QLabel * ratio_lbl = constructDataCellLabel(ratio, 3);
    layout->addWidget(ratio_lbl);
    newCell->setLayout(layout);

//    QString lbl_styleSheet = styleSheetColorMaker(QString("color"), foreground);
//    val_lbl->setStyleSheet(lbl_styleSheet);
//    ratio_lbl->setStyleSheet(lbl_styleSheet);

    QString bg_styleSheet = styleSheetColorMaker(QString("background-color"), background);
    newCell->setStyleSheet(bg_styleSheet);

    val_lbl->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    ratio_lbl->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    newCell->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    return newCell;
}

QLabel * ConfusionMatrixDiagram::constructDataCellLabel(const real & val, const int & prec) {
    QLabel * lbl = new QLabel();
    if (prec > 0) {
        lbl->setText(QString::number(val, 'g', prec));
    } else {
        lbl->setText(QString::number(static_cast<int>(val)));
    }
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setMargin(0);
    lbl->setScaledContents(true);
    return lbl;
}

QColor ConfusionMatrixDiagram::linearGradient(const real & val, const real & minVal, const real & maxVal, const QColor & lowColor, const QColor & highColor) {
    int r = static_cast<int>(linearGradientChannel(val, minVal, maxVal, lowColor.redF(), highColor.redF()) * 255.0);
    int g = static_cast<int>(linearGradientChannel(val, minVal, maxVal, lowColor.greenF(), highColor.greenF()) * 255.0);
    int b = static_cast<int>(linearGradientChannel(val, minVal, maxVal, lowColor.blueF(), highColor.blueF()) * 255.0);
    int a = static_cast<int>(linearGradientChannel(val, minVal, maxVal, lowColor.alphaF(), highColor.alphaF()) * 255.0);

    r = r < 0 ? 0 : r;
    g = g < 0 ? 0 : g;
    b = b < 0 ? 0 : b;
    a = a < 0 ? 0 : a;

    r = r > 255 ? 255 : r;
    g = g > 255 ? 255 : g;
    b = b > 255 ? 255 : b;
    a = a > 255 ? 255 : a;

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

    r = r < 0 ? 0 : r;
    g = g < 0 ? 0 : g;
    b = b < 0 ? 0 : b;
    a = a < 0 ? 0 : a;

    r = r > 255 ? 255 : r;
    g = g > 255 ? 255 : g;
    b = b > 255 ? 255 : b;
    a = a > 255 ? 255 : a;

    return QColor(r, g, b, a);
}

double ConfusionMatrixDiagram::linearGradientChannel(const real & ratio, const double & lowColor, const double & highColor) {
    real m_ratio = ratio;
    if (m_ratio < 0) m_ratio = 0;
    if (m_ratio > 1) m_ratio = 1;

    return ( m_ratio * (highColor - lowColor) ) + lowColor;
}




