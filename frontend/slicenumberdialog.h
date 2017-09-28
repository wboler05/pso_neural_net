#ifndef SLICENUMBERDIALOG_H
#define SLICENUMBERDIALOG_H

#include <QWidget>

namespace Ui {
class SliceNumberDialog;
}

class SliceNumberDialog : public QWidget
{
    Q_OBJECT

public:
    explicit SliceNumberDialog(const int & defaultSlices, QWidget *parent = 0);
    ~SliceNumberDialog();

private:
    Ui::SliceNumberDialog *ui;

protected slots:
    void accepted();
    void cancelled();

signals:
    void slicesUpdated(const int &);
};

#endif // SLICENUMBERDIALOG_H
