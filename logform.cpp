#include "logform.h"
#include "ui_logform.h"
#include <QDateTime>


LogForm::LogForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LogForm)
{
    ui->setupUi(this);

    connect(ui->clean, &QPushButton::clicked, this, [this]{ui->textBrowser->clear();});
}

LogForm::~LogForm()
{
    delete ui;
}

void LogForm::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // 刷新通过Qt Designer设计的UI部分的翻译
        ui->retranslateUi(this);

        // 然后手动更新所有动态创建的组件的文本
    }

    QWidget::changeEvent(event); // 调用基类处理
}

void LogForm::onLogInfo(QString arg)
{

    ui->textBrowser->append(QString("%1  %2").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"), arg));
}
