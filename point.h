#ifndef POINT_H
#define POINT_H

#include <QPainter>
#include <QGraphicsItem>
#include <QDebug>

class Point : public QGraphicsObject
{
public:
    Point(qreal x, qreal y);
    qreal x,y;
    double xval,yval;
    QString groupName;
    QString sampleName;
    int groupID;
    int sampleID;
    QColor color;
    bool hovered, clicked;

    QRectF boundingRect() const;
    void setColor(QColor colour);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

};

#endif // POINT_H
