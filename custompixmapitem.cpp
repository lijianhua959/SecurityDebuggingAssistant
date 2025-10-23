#include "custompixmapitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsView>

#include "settings.h"


CustomPixmapItem::CustomPixmapItem()
{
    mTranslate      = false;
    mLeftButton     = false;
    mRightButton    = false;
    mDrawPolygon    = false;
    mDoDraw         = false;
    mIsPointMove    = false;
    mIsPolygonMove  = false;
    mIsCalibration  = false;
    mIsPosReset     = false;

    mCounter        = -1;
    mAreaIndex      = 0;
    mGroupIndex     = 0;
    mZValue         = 0;
    mScaleFactor    = 1;

    mPolygonF << QPointF(0, 0) << QPointF(0, 0) << QPointF(0, 0) << QPointF(0, 0);

    int i = 1;
    for (auto &var : mPolygonItems)
    {
        var.hide();
        var.setPen(QPen(QColor(0, 170, 0)));
        var.setBrush(QBrush(QColor(0, 170, 0, 50)));
        var.setPolygon(mPolygonF);
        var.setParentItem(this);
        if(i < 9) var.setText(QString::number(i++));
    }
    mPolygonItems[0].setAcceptHoverEvents(true);

    for (auto &var : mLineItems)
    {
        var.setPen(QPen(QColor(0, 170, 255), 2));
        var.setParentItem(this);
        var.hide();
    }
    mLineItems[3].setPen(QPen(QColor(0, 170, 255), 2, Qt::DotLine));

    for (auto &var : mTextItems)
    {
        var.hide();
        var.setBrush(QBrush(QColor(255, 0, 255)));
        var.setFont(QFont("Arial", 40));
        var.setParentItem(this);
        var.setText("×");
    }

    this->setAcceptHoverEvents(true);
}

void CustomPixmapItem::setGroupNumber(int num)
{
    mGroupIndex = num;

    hideLineItems();
    hideTextItems();

    for (int i = 0; i < 8; ++i)
    {
        mPolygonItems[i].hide();

        for (int j = 0, k = 0; j < 4; ++j)
        {
            mPolygonF[j].rx() = ARG.Group[num].ROI[i].srcArea[k++];
            mPolygonF[j].ry() = ARG.Group[num].ROI[i].srcArea[k++];
        }

        mPolygonItems[i].setPolygon(mPolygonF);
        if((mPolygonF[0] == mPolygonF[2]) || mIsCalibration) continue;

        mPolygonItems[i].show();
    }
}

void CustomPixmapItem::setAreaNumber(int arg)
{
    hideTextItems();
    hideLineItems();
    if(arg < 8 && mAreaIndex < 8)
    {
        auto pg = mPolygonItems[mAreaIndex].polygon();
        if(pg[0] != pg[2]) mPolygonItems[mAreaIndex].show();
    }
    mPolygonItems[mAreaIndex].setAcceptHoverEvents(false);

    mAreaIndex = arg;
    mPolygonItems[arg].setZValue(++mZValue);
    mPolygonItems[arg].setAcceptHoverEvents(true);
}

void CustomPixmapItem::restoreArea()
{
    for (int i = 0, k = 0; i < 4; ++i)
    {
        mPolygonF[i].rx() = ARG.Group[mGroupIndex].ROI[mAreaIndex].srcArea[k++];
        mPolygonF[i].ry() = ARG.Group[mGroupIndex].ROI[mAreaIndex].srcArea[k++];
    }
    //
    mPolygonItems[mAreaIndex].setPolygon(mPolygonF);
    if(mPolygonF[0] == mPolygonF[2])
    {
        mPolygonItems[mAreaIndex].hide();
    }
    else
    {
        mPolygonItems[mAreaIndex].show();
    }
}

void CustomPixmapItem::setDrawModel(int arg)
{
    mDrawPolygon = arg;
    hideLineItems();
}

void CustomPixmapItem::setEditModel(bool arg)
{
    mDoDraw = arg;
    hideLineItems();
}

void CustomPixmapItem::setCalibrationModel(bool arg)
{
    mIsCalibration = arg;
    hideLineItems();

    if(arg)
    {
        for (auto &var : mPolygonItems) var.hide();
        mPolygonItems[8].show();
    }
    else
    {
        for (auto &var : mPolygonItems)
        {
            auto pg = var.polygon();
            if(pg[0] != pg[2]) var.show();
        }
        mPolygonItems[8].hide();
    }
}

void CustomPixmapItem::setPointMoveEnable(bool arg)
{
    if(!mDoDraw) return;

    mIsPointMove = arg;
    mPolygonItems[mAreaIndex].setPointMoveEnable(arg);
}

void CustomPixmapItem::setPolygonMoveEnable(bool arg)
{
    if(!mDoDraw) return;

    if(mIsPosReset)
    {
        mIsPosReset = false;
        mPolygonItems[mAreaIndex].setPolygon(mPolygonItems[mAreaIndex].mapToParent(mPolygonItems[mAreaIndex].polygon()));
        mPolygonItems[mAreaIndex].setPos(0, 0);
    }
    mIsPolygonMove = arg;
    mPolygonItems[mAreaIndex].setMoveEnable(arg);
}

void CustomPixmapItem::loadDrawArea()
{
    auto polyg = mPolygonItems[mAreaIndex].polygon();
    for (int i = 0, k = 0; i < 4; ++i, k += 2)
    {
        ARG.Group[mGroupIndex].ROI[mAreaIndex].srcArea[k  ] = (int)polyg[i].x();
        ARG.Group[mGroupIndex].ROI[mAreaIndex].srcArea[k+1] = (int)polyg[i].y();
    }
}

void CustomPixmapItem::loadCalibrationArea()
{
    auto polyg = mPolygonItems[8].polygon();
    for (int i = 0, k = 0; i < 4; ++i)
    {
        ARG.srcCalibrationParams[k++] = polyg[i].x();
        ARG.srcCalibrationParams[k++] = polyg[i].y();
    }
}

void CustomPixmapItem::setColor(QColor color)
{
    config.colors[mAreaIndex] = color;

    mPolygonItems[mAreaIndex].setPen(QPen(color, 2));
    color.setAlpha(50);
    mPolygonItems[mAreaIndex].setBrush(QBrush(color));

    mPolygonItems[mAreaIndex].update();
}

void CustomPixmapItem::alarmZone(uint16_t *ptr, uint8_t *atr)
{
    for (uint16_t i = 0; i < 8; ++i)
    {
        if(ARG.Group[mGroupIndex].ROI[i].enable)
        {
            mPolygonItems[i].setPen(QPen(config.colors[i]));
            auto color = config.colors[i];
            color.setAlpha(50);
            mPolygonItems[i].setBrush(QBrush(color));
        }
        else
        {
            mPolygonItems[i].setPen(QPen(QColor(255,255,255)));
            mPolygonItems[i].setBrush(QBrush(QColor(255,255,255, 50)));
        }
    }

    for (uint16_t i = 0; i < ptr[0]; ++i)
    {
        if(atr[i+1] == 1)
        {
            mPolygonItems[ptr[i+1]-1].setPen(QPen(QColor(255,0,0)));
            mPolygonItems[ptr[i+1]-1].setBrush(QBrush(QColor(255,0,0,50)));
        }
        else if(atr[i+1] == 2)
        {
            mPolygonItems[ptr[i+1]-1].setPen(QPen(QColor(170,0,255)));
            mPolygonItems[ptr[i+1]-1].setBrush(QBrush(QColor(170,0,255,50)));
        }
    }
}

void CustomPixmapItem::anomalyPoint(std::vector<int>& vp)
{
    auto polyg = mPolygonItems[mAreaIndex].polygon();

    for (auto &var : mTextItems) var.hide();
    for (auto var : vp)
    {
        mTextItems[var].setPos(polyg[var] - QPointF(15.5, 28.5));
        mTextItems[var].show();
    }
}

bool CustomPixmapItem::isAreaChange()
{
    auto polyg = mPolygonItems[mAreaIndex].polygon();
    for (int i = 0, k = 0; i < 4; ++i, k += 2)
    {
        if(((int)polyg[i].x() != ARG.Group[mGroupIndex].ROI[mAreaIndex].srcArea[k])
            || ((int)polyg[i].y() != ARG.Group[mGroupIndex].ROI[mAreaIndex].srcArea[k+1])
            )

        {
            return true;
        }
    }

    return false;
}

bool CustomPixmapItem::isQuadrilateral(QPointF p1, QPointF p2, QPointF p3, QPointF p4)
{
    // 计算四个三点组的叉积
    int cross1 = (p2.x() - p1.x()) * (p3.y() - p1.y()) - (p2.y() - p1.y()) * (p3.x() - p1.x());
    int cross2 = (p3.x() - p2.x()) * (p4.y() - p2.y()) - (p3.y() - p2.y()) * (p4.x() - p2.x());
    int cross3 = (p4.x() - p3.x()) * (p1.y() - p3.y()) - (p4.y() - p3.y()) * (p1.x() - p3.x());
    int cross4 = (p1.x() - p4.x()) * (p2.y() - p4.y()) - (p1.y() - p4.y()) * (p2.x() - p4.x());

    // 检查是否四点共线、交叉
    if ((cross1 == 0 && cross2 == 0)
        || (((cross1 > 0 && cross2 < 0) || (cross1 < 0 && cross2 > 0)) &&
            ((cross3 > 0 && cross4 < 0) || (cross3 < 0 && cross4 > 0)))
        )
        return false;

    // 计算四个三点组的叉积
    cross1 = (p3.x() - p2.x()) * (p4.y() - p2.y()) - (p3.y() - p2.y()) * (p4.x() - p2.x());
    cross2 = (p4.x() - p3.x()) * (p1.y() - p3.y()) - (p4.y() - p3.y()) * (p1.x() - p3.x());
    cross3 = (p1.x() - p4.x()) * (p2.y() - p4.y()) - (p1.y() - p4.y()) * (p2.x() - p4.x());
    cross4 = (p2.x() - p1.x()) * (p3.y() - p1.y()) - (p2.y() - p1.y()) * (p3.x() - p1.x());

    // 判断交叉
    return !(((cross1 > 0 && cross2 < 0) || (cross1 < 0 && cross2 > 0)) &&
             ((cross3 > 0 && cross4 < 0) || (cross3 < 0 && cross4 > 0)));

    // // 检查是否有任何叉积为零（三点共线）
    // if (cross1 == 0 || cross2 == 0 || cross3 == 0 || cross4 == 0)
    //     return false;

    // // 检查所有叉积是否同号
    // bool positive = (cross1 > 0);
    // return (cross2 > 0) == positive &&
    //        (cross3 > 0) == positive &&
    //        (cross4 > 0) == positive;
}

void CustomPixmapItem::hideLineItems()
{
    mCounter = -1;
    for (auto &var : mLineItems) var.hide();
}

void CustomPixmapItem::hideTextItems()
{
    for (auto &var : mTextItems) var.hide();
}

void CustomPixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(!mDoDraw) return;

    mPoint = event->pos();
    if(event->button() == Qt::RightButton) // 与绘制四边形相关
    {
        mRightButton = true;

        if(mPoint.x() < 0)      mPoint.rx() = 0;
        if(mPoint.y() < 0)      mPoint.ry() = 0;
        if(mPoint.x() > 1600)    mPoint.rx() = 1600;
        if(mPoint.y() > 1200)    mPoint.ry() = 1200;

        if(mDrawPolygon)
        {
            ++mCounter;
            if(mCounter == 0) mPolygonItems[mAreaIndex].hide();
            mPolygonF[mCounter] = mPoint;
            if(mCounter < 3)
            {
                mLineF.setPoints(mPoint, mPoint);
                mLineItems[mCounter].setLine(mLineF);
                mLineItems[mCounter].show();
                if(mCounter == 2)
                {
                    mLineItems[3].setLine(0,0,0,0);
                    mLineItems[3].show();
                }
            }
        }

        return;
    }

    if(event->button() == Qt::LeftButton) // 与四边形（点）移动相关
    {
        mLeftButton = true;

        if(mIsPolygonMove && mPolygonItems[mAreaIndex].isSelect)
        {
            mTranslate = true;
            return;
        }

        if(mIsPointMove && mPolygonItems[mAreaIndex].isPointSelect)
        {
            mPolygonF = mPolygonItems[mAreaIndex].polygon();
            mTranslate = true;
            return;
        }
    }

}

void CustomPixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(!mDoDraw) return;

    if(event->button() == Qt::RightButton) // 与绘制四边形相关
    {
        mRightButton = false;

        if(mDrawPolygon)
        {
            if(mCounter == 3)
            {
                if(mIsQuad)
                {
                    hideLineItems();
                    mPolygonItems[mAreaIndex].setPolygon(mPolygonF);
                    mPolygonItems[mAreaIndex].show();
                }
                else
                {
                    mCounter -= 1;
                }
            }
        }

        return;
    }

    if(event->button() == Qt::LeftButton) // 与四边形（点）移动相关
    {
        if(mIsPosReset)
        {
            mIsPosReset = false;
            mPolygonItems[mAreaIndex].setPolygon(mPolygonItems[mAreaIndex].mapToParent(mPolygonItems[mAreaIndex].polygon()));
            mPolygonItems[mAreaIndex].setPos(0, 0);
        }

        mLeftButton = false;
        mTranslate  = false;
    }
}

void CustomPixmapItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(!mDoDraw) return;

    if(mLeftButton)
    {
        if(mIsPolygonMove)
        {
            if(mTranslate)
            {
                auto pot = event->pos() - mPoint;
                mPolygonItems[mAreaIndex].moveBy(pot.rx(), pot.ry());
                // 区域判定
                if(this->boundingRect().contains(mPolygonItems[mAreaIndex].mapRectToParent(mPolygonItems[mAreaIndex].boundingRect())))
                {
                    mPoint = event->pos();
                }
                else
                {
                    mPolygonItems[mAreaIndex].moveBy(-pot.rx(), -pot.ry());
                }
                mIsPosReset = true;
            }

            return;
        }

        if(mIsPointMove)
        {
            if(mTranslate)
            {
                // 区域判定
                if((mPolygonItems[mAreaIndex].pointIndex > -1) && this->boundingRect().contains(event->pos()))
                {
                    mPolygonF[mPolygonItems[mAreaIndex].pointIndex] = event->pos();
                    // 多边形判定
                    if(isQuadrilateral(mPolygonF[0], mPolygonF[1], mPolygonF[2], mPolygonF[3]))
                    {
                        mPolygonItems[mAreaIndex].setPolygon(mPolygonF);
                    }
                }
            }

            return;
        }

        if(!mTranslate) // IR图像移动
        {
            auto pot = (event->pos() - mPoint) * mScaleFactor;
            this->moveBy(pot.rx(), pot.ry());
        }

        return;
    }

    if(mRightButton)
    {
        if(!mDrawPolygon)
        {
            auto point = event->pos();

            if(point.x() < 0)       point.rx() = 0;
            if(point.y() < 0)       point.ry() = 0;
            if(point.x() > 1600)    point.rx() = 1600;
            if(point.y() > 1200)    point.ry() = 1200;

            if(point != mPoint)
            {
                if(point.x() > mPoint.x())
                {
                    mPolygonF[0].rx() = mPoint.x();
                    mPolygonF[2].rx() = (point.x() < 1600) ? point.x() : 1600;
                }
                else
                {
                    mPolygonF[2].rx() = mPoint.x();
                    mPolygonF[0].rx() = (point.x() > 0) ? point.x() : 0;
                }

                if(point.y() > mPoint.y())
                {
                    mPolygonF[0].ry() = mPoint.y();
                    mPolygonF[2].ry() = (point.y() < 1200) ? point.y() : 1200;
                }
                else
                {
                    mPolygonF[2].ry() = mPoint.y();
                    mPolygonF[0].ry() = (point.y() > 0) ? point.y() : 0;
                }

                mPolygonF[1].rx() = mPolygonF[2].x();
                mPolygonF[1].ry() = mPolygonF[0].y();
                mPolygonF[3].rx() = mPolygonF[0].x();
                mPolygonF[3].ry() = mPolygonF[2].y();

                mPolygonItems[mAreaIndex].setPolygon(mPolygonF);
                mPolygonItems[mAreaIndex].show();
            }
        }
    }
}

void CustomPixmapItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{

    QGraphicsPixmapItem::hoverEnterEvent(event);
}

void CustomPixmapItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{

    QGraphicsPixmapItem::hoverLeaveEvent(event);
}

void CustomPixmapItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if(mDoDraw)
    {
        if(mDrawPolygon)
        {
            if(mCounter > -1 && mCounter < 3)
            {
                if(mCounter == 2)
                {
                    if(!isQuadrilateral(mPolygonF[0], mPolygonF[1], mPolygonF[2], event->pos()))
                    {
                        mIsQuad = false;
                        return;
                    }
                    mIsQuad = true;
                    mLineItems[3].setLine(QLineF(event->pos(), mLineItems[0].line().p1()));
                }

                mLineF.setP2(event->pos());
                mLineItems[mCounter].setLine(mLineF);
            }
        }
    }
}

void CustomPixmapItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if(!mDoDraw) return;

    //选点缩放 = 缩放+移动
    auto pot = event->pos() * mScaleFactor * 0.1;
    if(event->delta() > 0)
    {
        mScaleFactor *= 1.1;
        this->setScale(mScaleFactor);
        this->moveBy(-pot.rx(), -pot.ry());
    }
    else
    {
        mScaleFactor *= 0.9;
        this->setScale(mScaleFactor);
        this->moveBy(pot.rx(), pot.ry());
    }
}
