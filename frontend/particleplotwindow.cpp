#include "particleplotwindow.h"
#include "ui_particleplotwindow.h"

ParticlePlotWindow::ParticlePlotWindow(const std::shared_ptr<std::vector<NeuralPso::NeuralParticle>> & particles, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ParticlePlotWindow)
{
    ui->setupUi(this);
    ui->particlePlot->setParticles(particles);
    scanDimensions();

    guiUpdate = new QTimer();
    connect(guiUpdate, SIGNAL(timeout()), this, SLOT(updatePlot()));
    guiUpdate->setInterval(67);
    guiUpdate->start();

    connectListeners();
}

ParticlePlotWindow::~ParticlePlotWindow()
{
    delete ui;
}

void ParticlePlotWindow::connectListeners() {
    connect(ui->close_btn, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->cycle_btn, SIGNAL(clicked()), this, SLOT(resetCycleTimer()));
}

void ParticlePlotWindow::updatePlot() {
    ui->particlePlot->updatePlot(static_cast<size_t>(ui->firstDimension_sb->value()),
                                 static_cast<size_t>(ui->secondDimension_sb->value()),
                                 static_cast<size_t>(ui->thirdDimension_sb->value()));
}

void ParticlePlotWindow::scanDimensions() {
    if (ui->particlePlot->particles() == nullptr) return;
    if (ui->particlePlot->particles()->size() == 0) return;
    NeuralPso::NeuralParticle & particle = (*ui->particlePlot->particles())[0];
    size_t layer = ui->firstDimension_sb->value();
    size_t left_node = ui->secondDimension_sb->value();

    ui->firstDimension_sb->setMinimum(0);
    ui->secondDimension_sb->setMinimum(0);
    ui->thirdDimension_sb->setMinimum(0);

    ui->firstDimension_sb->setMaximum(particle._x.size());
    if (layer < particle._x.size()) {
        ui->secondDimension_sb->setMaximum(particle._x[layer].size());
        if (left_node < particle._x[layer].size()) {
            ui->thirdDimension_sb->setMaximum(particle._x[layer][left_node].size());
        }
    }
}

void ParticlePlotWindow::resetCycleTimer() {
    toggleCycleBtn();

    if (ui->cycle_btn->isChecked()) {
        if (!cycleUpdate) {
            cycleUpdate = new QTimer();
            connect(cycleUpdate, SIGNAL(timeout()), this, SLOT(cycleDimensions()));
        }
        cycleUpdate->stop();
        int cycle_rate = static_cast<int>(ui->cycle_rate_dsb->value() * 1000.0);
        if (cycle_rate > 0) {
            cycleUpdate->setInterval(cycle_rate);
            cycleUpdate->start();
        }
    } else {
        if (cycleUpdate) {
            cycleUpdate->stop();
        }
    }
}

void ParticlePlotWindow::toggleCycleBtn() {
    //ui->cycle_btn->setChecked(!ui->cycle_btn->isChecked());
}

void ParticlePlotWindow::cycleDimensions() {
    if (ui->particlePlot->particles() == nullptr) return;
    if (ui->particlePlot->particles()->size() == 0) return;
    NeuralPso::NeuralParticle & particle = (*ui->particlePlot->particles())[0];
    size_t layer = static_cast<size_t>(ui->firstDimension_sb->value());
    size_t left_node = static_cast<size_t>(ui->secondDimension_sb->value());
    size_t right_node = static_cast<size_t>(ui->thirdDimension_sb->value());

    if (layer < particle._x.size()) {
        if (left_node < particle._x[layer].size()) {
            if (right_node < particle._x[layer][left_node].size()) {
                right_node++;
            }
        } else {
            return;
        }
    } else {
        return;
    }

    if (right_node >= particle._x[layer][left_node].size()) {
        left_node++;
        right_node = 0;
    }
    if (left_node >= particle._x[layer].size()) {
        layer++;
        left_node = 0;
    }
    if (layer >= particle._x.size()) {
        layer = 0;
    }

    ui->firstDimension_sb->setValue(static_cast<int>(layer));
    ui->secondDimension_sb->setValue(static_cast<int>(left_node));
    ui->thirdDimension_sb->setValue(static_cast<int>(right_node));

    scanDimensions();
}
