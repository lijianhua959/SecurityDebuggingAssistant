#ifndef CUSTOMPIXMAPITEM_H
#define CUSTOMPIXMAPITEM_H

#include <QGraphicsPixmapItem>
#include "custompolygonitem.h"


class CustomPixmapItem : public QGraphicsPixmapItem
{
public:
    CustomPixmapItem();

    void setGroupNumber(int);
    void setAreaNumber(int );
    void restoreArea();
    void setDrawModel(int );
    void setEditModel(bool );
    void setCalibrationModel(bool );
    void setPointMoveEnable(bool );
    void setPolygonMoveEnable(bool );
    void loadDrawArea();
    void loadCalibrationArea();

    void setColor(QColor);
    void alarmZone(uint16_t*, uint8_t *aptr);
    void anomalyPoint(std::vector<int> &);

    bool isAreaChange();

private:
    // 判断否为四边形
    bool isQuadrilateral(QPointF p1, QPointF p2, QPointF p3, QPointF p4);
    void hideLineItems();
    void hideTextItems();


protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void wheelEvent(QGraphicsSceneWheelEvent *event) override;

private:
    bool                        mTranslate;
    bool                        mLeftButton;
    bool                        mRightButton;
    bool                        mDrawPolygon;
    bool                        mDoDraw;
    bool                        mIsQuad;
    bool                        mIsPointMove;
    bool                        mIsPolygonMove;
    bool                        mIsCalibration;
    bool                        mIsPosReset;

    int                         mGroupIndex;
    int                         mAreaIndex;
    int                         mCounter;

    qreal                       mScaleFactor;
    qreal                       mZValue;

    QPointF                     mPoint;
    QPolygonF                   mPolygonF;
    CustomPolygonItem           mPolygonItems[9];
    QLineF                      mLineF;
    QGraphicsLineItem           mLineItems[4];
    QGraphicsSimpleTextItem     mTextItems[4];

};

#endif // CUSTOMPIXMAPITEM_H
