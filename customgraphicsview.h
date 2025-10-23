#ifndef CUSTOMGRAPHICSVIEW_H
#define CUSTOMGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QLabel>

#include "custompixmapitem.h"


class CustomGraphicsView : public QGraphicsView
{
public:
    CustomGraphicsView(QWidget *parent = nullptr);
    void setItem(CustomPixmapItem& item);

protected:
    void resizeEvent(QResizeEvent *event) override;


private:
    CustomPixmapItem    *mItem;

};

#endif // CUSTOMGRAPHICSVIEW_H
