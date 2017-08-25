#ifndef ABOUTCONFUSIONMATRIXDIALOG_H
#define ABOUTCONFUSIONMATRIXDIALOG_H

#include <QDialog>

namespace Ui {
class AboutConfusionMatrixDialog;
}

class AboutConfusionMatrixDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutConfusionMatrixDialog(QWidget *parent = 0);
    ~AboutConfusionMatrixDialog();

private:
    Ui::AboutConfusionMatrixDialog *ui;
};

#endif // ABOUTCONFUSIONMATRIXDIALOG_H
