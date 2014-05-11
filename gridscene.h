#ifndef GRIDSCENE_H
#define GRIDSCENE_H

#include <QGraphicsScene>
#include <QVarLengthArray>
#include <QDebug>
#include <QPainter>

class GridScene : public QGraphicsScene
{
    Q_OBJECT
public:
    GridScene(){}

protected:
    void drawBackground(QPainter *painter, const QRectF &rect)
    {
        const int gridSize = 100;

        qreal left = int(rect.left()) - (int(rect.left()) % gridSize);
        qreal top = int(rect.top()) - (int(rect.top()) % gridSize);

        QVarLengthArray<QLineF, 100> lines;

        for (qreal x = left; x < rect.right(); x += gridSize)
            lines.append(QLineF(x, rect.top(), x, rect.bottom()));
        for (qreal y = top; y < rect.bottom(); y += gridSize)
            lines.append(QLineF(rect.left(), y, rect.right(), y));

        painter->setPen(Qt::lightGray);
        painter->drawLines(lines.data(), lines.size());
    }
};

#endif // GRIDSCENE_H
