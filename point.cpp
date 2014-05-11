#include "point.h"
#include "pcoadialog.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

Point::Point(qreal x, qreal y) : QGraphicsObject()
{
    this->setAcceptHoverEvents(true);
    this->x=x;
    this->y=y;
    hovered= false;
    clicked=false;

}

QRectF Point::boundingRect() const
{
    return QRectF(x,y,10,10);
}

void Point::setColor(QColor colour)
{
    color = colour;
}

void Point::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

    QRectF rec= boundingRect();
    QBrush redBrush(Qt::red);

    QBrush dynamicBrush(color);

    QString label = "Sample Name : ";
    label.append(sampleName);label.append("\n");label.append("Group Name : ");label.append(groupName);
    if (hovered)
    {
        painter->setBrush(redBrush);
        painter->setPen(QPen(Qt::black,2,Qt::SolidLine,Qt::SquareCap,Qt::RoundJoin));
        painter->drawEllipse(rec);
        this->setZValue(2);
        ((PCOADialog*)(widget->parent()->parent()->parent()))->getTextBrowser1()->clear();
        ((PCOADialog*)(widget->parent()->parent()->parent()))->getTextBrowser1()->append(label);
    }
    else
    {
        painter->setBrush(dynamicBrush);
        painter->setPen(QPen(Qt::black,1,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        painter->drawEllipse(rec);
        ((PCOADialog*)(widget->parent()->parent()->parent()))->getTextBrowser1()->clear();
        this->setZValue(0);
    }

    if (clicked)
    {
        ((PCOADialog*)(widget->parent()->parent()->parent()))->getTextBrowser2()->clear();

        QString label = "Sample Name : ";
        label.append(sampleName);label.append("\n");label.append("Group Name : ");label.append(groupName);
        painter->setBrush(dynamicBrush);
        painter->setPen(QPen(Qt::black,2,Qt::SolidLine,Qt::SquareCap,Qt::RoundJoin));
        painter->drawEllipse(rec);
        ((PCOADialog*)(widget->parent()->parent()->parent()))->getTextBrowser2()->append(label);
    }

}

void Point::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    hovered= true;

    update();
    QGraphicsItem::hoverEnterEvent(event);
}

void Point::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    hovered= false;
    //clicked= false;

    update();
    QGraphicsItem::hoverLeaveEvent(event);
}

void Point::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    clicked= true;

    update();
    QGraphicsItem::mousePressEvent(event);
}
