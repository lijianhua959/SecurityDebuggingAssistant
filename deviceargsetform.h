#ifndef DEVICEARGSETFORM_H
#define DEVICEARGSETFORM_H

#include <QWidget>

namespace Ui {
class DeviceArgSetForm;
}

class DeviceArgSetForm : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceArgSetForm(QWidget *parent = nullptr);
    ~DeviceArgSetForm();

    void setArgUI();
    void setExposureLevel(int);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void changeEvent(QEvent *event);

signals:
    void saveDeviceConfig();
    void setExposure(int );
    void setFrameRate(int );
    void setSpatialFilter(bool, int);
    void setTimeFilter(bool, int);
    void setTimeFilter_(bool, int);
    void setFlyPixeFilter(bool, int);
    void setConfidenceFilter(bool, int);


private slots:
    void on_pushButton_clicked();

private:
    Ui::DeviceArgSetForm    *ui;

};

#endif // DEVICEARGSETFORM_H
