#include "customellipseitem.h"
#include <QPen>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>


CustomEllipseItem::CustomEllipseItem(bool& arg1, int & arg2) : mCursorEnter(arg1), mIndex(arg2)
{
    mIsPointMove = false;
    mCursorEnter = false;
    mNumber = -1;

    this->setPen(QPen(QColor(119, 179, 0, 255)));
    this->setBrush(QBrush(QColor(119, 179, 0, 255)));

    this->setAcceptHoverEvents(true);
}

void CustomEllipseItem::setNumber(int num)
{
    mNumber = num;
}

void CustomEllipseItem::setPointMoveEnable(bool arg)
{
    mIsPointMove = arg;
    if(arg && mCursorEnter)
    {
        if(QGuiApplication::overrideCursor() == nullptr)
            QGuiApplication::setOverrideCursor(Qt::CrossCursor);
        else
            QGuiApplication::changeOverrideCursor(Qt::CrossCursor);
    }
    else
    {
        mCursorEnter = false;
        QGuiApplication::restoreOverrideCursor();
    }
}

void CustomEllipseItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    mIndex = mNumber;
    mCursorEnter = true;

    if(mIsPointMove)
    {
        if(QGuiApplication::overrideCursor() == nullptr)
            QGuiApplication::setOverrideCursor(Qt::CrossCursor);
        else
            QGuiApplication::changeOverrideCursor(Qt::CrossCursor);
    }

    QGraphicsEllipseItem::hoverEnterEvent(event);
}

void CustomEllipseItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    mIndex = -1;
    mCursorEnter = false;
    QGuiApplication::restoreOverrideCursor();

    QGraphicsEllipseItem::hoverLeaveEvent(event);
}
