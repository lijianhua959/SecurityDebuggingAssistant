#include "customgraphicsview.h"
#include <QLayout>
#include <QMouseEvent>


CustomGraphicsView::CustomGraphicsView(QWidget *parent) : QGraphicsView(parent)
{
    // 设置无边框，防止闪烁
    this->setWindowFlags(Qt::WindowType::FramelessWindowHint);
    // 启用抗锯齿
    this->setRenderHint(QPainter::Antialiasing);
    // 关闭滚动条
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 设置视口更新模式
    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    // 添加场景
    this->setScene(new QGraphicsScene(this));
    // 设置场景背景颜色
    this->scene()->setBackgroundBrush(QColor(0,0,0,255));
}

void CustomGraphicsView::setItem(CustomPixmapItem &item)
{
    mItem = &item;
    this->scene()->addItem(&item);

    // 重新自适应窗口
    this->fitInView(mItem, Qt::KeepAspectRatio);
}

void CustomGraphicsView::resizeEvent(QResizeEvent *event)
{
    // 调整视图，使图像填充整个窗口（保持宽高比）
    fitInView(mItem, Qt::KeepAspectRatio);

    // 事件向下传递
    QGraphicsView::resizeEvent(event);
}
