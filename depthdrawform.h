#ifndef DEPTHDRAWFORM_H
#define DEPTHDRAWFORM_H

#include <QWidget>
#include <QGraphicsPixmapItem>
#include "settings.h"


namespace Ui {
class DepthDrawForm;
}

class DepthDrawForm : public QWidget
{
    Q_OBJECT

public:
    explicit DepthDrawForm(QWidget *parent = nullptr);
    ~DepthDrawForm();

    void setData(uint16_t* );

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);
    void changeEvent(QEvent *event);

private:
    Ui::DepthDrawForm   *ui;
    QRgb                *mDepthPixels;
    QRgb                mColorMap[1022];
    QImage              mDepthImage{TOF_PIX_COLS, TOF_PIX_ROWS, QImage::Format_RGBA8888};
    QGraphicsPixmapItem mDepthItem;
};

#endif // DEPTHDRAWFORM_H
