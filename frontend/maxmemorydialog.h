#ifndef MAXMEMORYDIALOG_H
#define MAXMEMORYDIALOG_H

#include <QWidget>
#include <cmath>

namespace Ui {
class MaxMemoryDialog;
}

class MaxMemoryDialog : public QWidget
{
    Q_OBJECT

public:
    explicit MaxMemoryDialog(const int & defaultBytes, QWidget *parent = 0);
    ~MaxMemoryDialog();

    enum WordType { Bytes=0, KBytes=1, MBytes=2, GBytes=3 };

    void setMaxBytes(const double & v);
    double convertBytes(double data, const WordType & fromType, const WordType & toType);
    double convertToBytes(double data, const WordType & type);

private:
    Ui::MaxMemoryDialog *ui;
    WordType _defaultType = Bytes;
    WordType _dialogSettingType = KBytes;

    void initializeComboBox();

protected slots:
    void accepted();
    void cancelled();

signals:
    void maxBytesUpdated(const int &);
};

Q_DECLARE_METATYPE(MaxMemoryDialog::WordType)

#endif // MAXMEMORYDIALOG_H
