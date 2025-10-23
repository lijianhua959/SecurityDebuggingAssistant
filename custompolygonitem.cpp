#include "custompolygonitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QCursor>
#include <QPen>
 #include <QFont>


CustomPolygonItem::CustomPolygonItem()
{
    mKeyShift = false;
    mKeyControl = false;
    isSelect = false;
    isPointSelect = false;

    pointIndex = -1;

    for (int i = 0; i < 4; ++i)
    {
        mEllipseItems[i].setNumber(i);
        mEllipseItems[i].setParentItem(this);
        mEllipseItems[i].hide();
    }

    mTextItem.setBrush(QBrush(QColor(0, 170, 255)));
    mTextItem.setFont(QFont("Times", 40));
    mTextItem.setParentItem(this);
}

void CustomPolygonItem::setMoveEnable(bool arg)
{
    mKeyControl = arg;

    if(arg && isSelect)
    {
        if(QGuiApplication::overrideCursor() == nullptr)
            QGuiApplication::setOverrideCursor(Qt::SizeAllCursor);
        else
            QGuiApplication::changeOverrideCursor(Qt::SizeAllCursor);
    }
    else
    {
        QGuiApplication::restoreOverrideCursor();
    }

}

void CustomPolygonItem::setPointMoveEnable(bool arg)
{
    mKeyShift = arg;

    for (auto & var : mEllipseItems)
    {
        arg ? var.show() : var.hide();
        var.setPointMoveEnable(arg);
    }
}

void CustomPolygonItem::setPolygon(const QPolygonF &polygon)
{
    for (int i = 0; i < 4; ++i)
    {
        mEllipseItems[i].setRect(polygon[i].x()-8, polygon[i].y()-8, 16, 16);
    }
    mTextItem.setPos((polygon[0]+polygon[2])/2);
    QGraphicsPolygonItem::setPolygon(polygon);
}

void CustomPolygonItem::setText(QString arg)
{
    mTextItem.setText(arg);
}

void CustomPolygonItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isSelect = true;
    if(mKeyControl)
    {
        if(QGuiApplication::overrideCursor() == nullptr)
            QGuiApplication::setOverrideCursor(Qt::SizeAllCursor);
        else
            QGuiApplication::changeOverrideCursor(Qt::SizeAllCursor);
    }

    QGraphicsPolygonItem::hoverEnterEvent(event);
}

void CustomPolygonItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isSelect = false;
    QGuiApplication::restoreOverrideCursor();

    QGraphicsPolygonItem::hoverLeaveEvent(event);
}
