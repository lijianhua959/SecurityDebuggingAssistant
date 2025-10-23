#ifndef CUSTOMELLIPSEITEM_H
#define CUSTOMELLIPSEITEM_H

#include <QGraphicsEllipseItem>

class CustomEllipseItem : public QGraphicsEllipseItem
{

public:
    CustomEllipseItem(bool&, int &);
    void setNumber(int num);
    void setPointMoveEnable(bool );

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    bool                    mIsPointMove;
    bool&                   mCursorEnter;
    int&                    mIndex;
    int                     mNumber;

};

#endif // CUSTOMELLIPSEITEM_H
