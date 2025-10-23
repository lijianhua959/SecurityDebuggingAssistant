#ifndef CUSTOMPOLYGONITEM_H
#define CUSTOMPOLYGONITEM_H

#include "customellipseitem.h"
#include <QGraphicsPolygonItem>



class CustomPolygonItem : public QGraphicsPolygonItem
{
public:
    CustomPolygonItem();
    void setPointMoveEnable(bool );
    void setMoveEnable(bool );
    void setPolygon(const QPolygonF &polygon);
    void setText(QString );

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

public:
    bool                    isSelect;
    bool                    isPointSelect;
    int                     pointIndex;

private:
    bool                    mKeyShift;
    bool                    mKeyControl;
    QPointF                 mPressPoint;
    QGraphicsSimpleTextItem mTextItem;
    CustomEllipseItem       mEllipseItems[4]{{isPointSelect, pointIndex}, {isPointSelect, pointIndex}, {isPointSelect, pointIndex}, {isPointSelect, pointIndex}};

};

#endif // CUSTOMPOLYGONITEM_H
