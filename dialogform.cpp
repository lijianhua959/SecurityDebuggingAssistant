#include "dialogform.h"
#include "ui_dialogform.h"
#include <QApplication>
#include <QStyle>

QIcon DialogForm::icon;

DialogForm::DialogForm(int w, int h, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogForm)
{
    ui->setupUi(this);

    this->setWindowIcon(icon);
    this->resize(w, h);
    connect(ui->confirm, &QPushButton::clicked, this, [this]{ this->accept(); });
}

DialogForm::~DialogForm()
{
    delete ui;
}

QMessageBox::StandardButton DialogForm::critical(const QString &text, QString title)
{
    this->setWindowTitle(title);
    ui->ico->setPixmap(QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical)
                           .pixmap(48, 48));

    ui->text->setText(text);

    auto ret = this->exec();
    return ret ? QMessageBox::Ok : QMessageBox::Cancel;
}

QMessageBox::StandardButton DialogForm::warning(const QString &text, QString title)
{
    this->setWindowTitle(title);
    ui->ico->setPixmap(QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning)
                           .pixmap(48, 48));

    ui->text->setText(text);

    auto ret = this->exec();
    return ret ? QMessageBox::Ok : QMessageBox::Cancel;
}

QMessageBox::StandardButton DialogForm::question(const QString &text, QString title)
{
    this->setWindowTitle(title);
    ui->ico->setPixmap(QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion)
                           .pixmap(48, 48));

    ui->text->setText(text);

    auto ret = this->exec();
    return ret ? QMessageBox::Ok : QMessageBox::Cancel;
}

QMessageBox::StandardButton DialogForm::information(const QString &text, QString title)
{
    this->setWindowTitle(title);
    ui->ico->setPixmap(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation)
                           .pixmap(48, 48));

    ui->text->setText(text);

    auto ret = this->exec();
    return ret ? QMessageBox::Ok : QMessageBox::Cancel;
}

void DialogForm::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // 刷新通过Qt Designer设计的UI部分的翻译
        ui->retranslateUi(this);

        // 然后手动更新所有动态创建的组件的文本
    }

    QWidget::changeEvent(event); // 调用基类处理
}
