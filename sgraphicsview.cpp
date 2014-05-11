#include "sgraphicsview.h"
#include <QDebug>
#include <QScrollBar>
#include "point.h"

SGraphicsView::SGraphicsView(QWidget *parent)
{
    //this->setDragMode(QGraphicsView::ScrollHandDrag);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pointclicked = false;
    highlighted = false;
}

void SGraphicsView::getItemsAroundMouse()
{
    QList<QGraphicsItem *> inside;

    qreal w= this->width()/2;
    qreal h=this->height()/2;
    qreal x=p.x()-(w/2);
    qreal y=p.y()-(h/2);
    inside = this->scene()->items(QRectF(mapToScene(x,y),QSize(w,h)),Qt::ContainsItemShape );

    if(!inside.isEmpty())
    {
        Point * poin = (Point*)(inside.at(0));
        xaround = poin->xval;
        yaround = poin->yval;

        foreach(QGraphicsItem * it, inside)
        {
            Point * po = (Point*)it;
            if(po)
            {
                po->setColor(Qt::magenta);
                highlighted = true;
            }
        }
        if(itemAt(QPoint((int)x,(int)y)))
        {
            pointclicked =true;
            QGraphicsItem *clicked = itemAt(QPoint((int)x,(int)y));
            Point * clickedPoint = (Point*)clicked;

            xclick = clickedPoint->xval;
            yclick = clickedPoint->yval;
        }
        update();
    }

}

void SGraphicsView::wheelEvent(QWheelEvent *event)
{
    //getItemsAroundMouse();
    if (event->delta()<0)
    {
        //Zooming out
        p=event->pos();
        ms=mapToScene(p);
        emit specialZoomOut();
    }
}

void SGraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    p=event->pos();
    ms=mapToScene(p);
    getItemsAroundMouse();

    if (highlighted)
    {
        emit specialZoomIn();
    }
}
