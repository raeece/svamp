#ifndef SGRAPHICSVIEW_H
#define SGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QGraphicsItem>
#include "point.h"

class SGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    SGraphicsView(QWidget *parent = 0);
    double hval,vval;
    QPoint p;
    QPointF ms;
    double xaround,yaround;
    double xclick,yclick;
    bool pointclicked, highlighted;
signals:
    void specialZoomIn();
    void specialZoomOut();
    void scroll();

private:
    QPointF LastPoint;
    QList<QGraphicsItem *> visibleItems;

    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
    void getItemsAroundMouse();
};

#endif // SGRAPHICSVIEW_H
