#include "networksetform.h"
#include "ui_networksetform.h"

NetworkSetForm::NetworkSetForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NetworkSetForm)
{
    ui->setupUi(this);

    connect(ui->OK, &QPushButton::clicked, this, [this]{
        emit setNetwork(
            ui->DHCP->isChecked(),
            QString("%1.%2.%3.%4").arg(ui->i1->value()).arg(ui->i2->value()).arg(ui->i3->value()).arg(ui->i4->value()),
            QString("%1.%2.%3.%4").arg(ui->n1->value()).arg(ui->n2->value()).arg(ui->n3->value()).arg(ui->n4->value())
            );
    });

}

NetworkSetForm::~NetworkSetForm()
{
    delete ui;
}

void NetworkSetForm::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // 刷新通过Qt Designer设计的UI部分的翻译
        ui->retranslateUi(this);

        // 然后手动更新所有动态创建的组件的文本
    }

    QWidget::changeEvent(event); // 调用基类处理
}
