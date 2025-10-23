#ifndef DIALOGFORM_H
#define DIALOGFORM_H

#include <QDialog>
#include <QMessageBox>

namespace Ui {
class DialogForm;
}

class DialogForm : public QDialog
{
    Q_OBJECT

public:
    explicit DialogForm(int w = 340, int h = 120, QWidget *parent = nullptr);
    ~DialogForm();

    QMessageBox::StandardButton critical(const QString &text, QString title = tr("错误"));
    QMessageBox::StandardButton warning(const QString &text, QString title = tr("警告"));
    QMessageBox::StandardButton question(const QString &text, QString title = tr("待确认"));
    QMessageBox::StandardButton information(const QString &text, QString title = tr("提示"));


public:
    static QIcon icon;


protected:
    void changeEvent(QEvent *event);


private:
    Ui::DialogForm *ui;
    // DialogForm      *self;
};

#endif // DIALOGFORM_H
