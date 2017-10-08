#ifndef PARTICLEPLOTWINDOW_H
#define PARTICLEPLOTWINDOW_H

#include "particleplot.h"
#include <QMainWindow>
#include <QTimer>

namespace Ui {
class ParticlePlotWindow;
}

class ParticlePlotWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ParticlePlotWindow(const std::shared_ptr<std::vector<NeuralPso::NeuralParticle>> & particles, QWidget *parent = 0);
    ~ParticlePlotWindow();

    void connectListeners();
    void scanDimensions();

protected slots:
    void cycleDimensions();
    void resetCycleTimer();
    void updatePlot();

private:
    Ui::ParticlePlotWindow *ui;
    QPointer<QTimer> guiUpdate;
    QPointer<QTimer> cycleUpdate;

    void toggleCycleBtn();
};

#endif // PARTICLEPLOTWINDOW_H
