#include "depthdrawform.h"
#include "ui_depthdrawform.h"

DepthDrawForm::DepthDrawForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DepthDrawForm)
{
    ui->setupUi(this);

    mDepthPixels = reinterpret_cast<QRgb*>(mDepthImage.bits());

    int index = 0;
    for (int i = 0; i < 255; ++i, ++index)
    {
        mColorMap[index] = qRgba(0, i, 255, 255);
    }
    for (int i = 255; i > 0; --i, ++index)
    {
        mColorMap[index] = qRgba(0, 255, i, 255);
    }
    for (int i = 0; i < 255; ++i, ++index)
    {
        mColorMap[index] = qRgba(i, 255, 0, 255);
    }
    for (int i = 255; i > -1; --i, ++index)
    {
        mColorMap[index] = qRgba(255, i, 0, 255);
    }
    mColorMap[index] = qRgba(0, 0, 0, 255);

    // 图像数据赋初值
    for (int i = 0; i < TOF_PIX_NUMBER; ++i) mDepthPixels[i] = mColorMap[1020];
    mDepthItem.setPixmap(QPixmap::fromImage(mDepthImage));

    // 启用抗锯齿
    ui->depthView->setRenderHint(QPainter::Antialiasing);
    // 关闭滚动条
    ui->depthView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->depthView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 设置视口更新模式
    ui->depthView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    // 添加场景
    ui->depthView->setScene(new QGraphicsScene(this));
    // 设置场景背景颜色
    ui->depthView->scene()->setBackgroundBrush(QColor(0,0,0,255));
    // 设置图元
    ui->depthView->scene()->addItem(&mDepthItem);
}

DepthDrawForm::~DepthDrawForm()
{
    delete ui;
}

void DepthDrawForm::setData(uint16_t *ptr)
{
    for (int i = 0, index = 0; i < TOF_PIX_NUMBER; ++i)
    {
        index = (ptr[i] < 6000) ? ((ptr[i] > 0) ? (ptr[i] * 0.17) : 1021): ((ptr[i] < 60000) ? 1020 : 1021);

        mDepthPixels[i] = mColorMap[index];
    }

    mDepthItem.setPixmap(QPixmap::fromImage(mDepthImage));
    ui->depthView->update();
}

void DepthDrawForm::showEvent(QShowEvent *event)
{
    ui->depthView->fitInView(&mDepthItem, Qt::KeepAspectRatio);
}

void DepthDrawForm::resizeEvent(QResizeEvent *event)
{
    ui->depthView->fitInView(&mDepthItem, Qt::KeepAspectRatio);
}

void DepthDrawForm::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // 刷新通过Qt Designer设计的UI部分的翻译
        ui->retranslateUi(this);

        // 然后手动更新所有动态创建的组件的文本
    }

    QWidget::changeEvent(event); // 调用基类处理
}
