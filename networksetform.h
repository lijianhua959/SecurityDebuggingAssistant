#ifndef NETWORKSETFORM_H
#define NETWORKSETFORM_H

#include <QWidget>

namespace Ui {
class NetworkSetForm;
}

class NetworkSetForm : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkSetForm(QWidget *parent = nullptr);
    ~NetworkSetForm();

protected:
    void changeEvent(QEvent *event);

signals:
    void setNetwork(bool, QString, QString);

private:
    Ui::NetworkSetForm *ui;
};

#endif // NETWORKSETFORM_H
