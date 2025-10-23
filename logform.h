#ifndef LOGFORM_H
#define LOGFORM_H

#include <QWidget>

namespace Ui {
class LogForm;
}

class LogForm : public QWidget
{
    Q_OBJECT

public:
    explicit LogForm(QWidget *parent = nullptr);
    ~LogForm();

protected:
    void changeEvent(QEvent *event);

public slots:
    void onLogInfo(QString);


private:
    Ui::LogForm *ui;
};

#endif // LOGFORM_H
