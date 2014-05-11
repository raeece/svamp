/*
 *  SVAMP - Sequence Variation Analysis , Map, and Phylogeny
 *
 *  www.cbrc.kaust.edu.sa/svamp
 *
 *  (c) Lailatul Hidayah, RaeeceNaeem Ghazali
 *
 *  This software was written, in large part, at King Abdullah University of Science and Technology (KAUST)
 *
 *  This file is part of SVAMP.
 *
 *  SVAMP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SVAMP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SVAMP.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef TRACKGRAPH_H
#define TRACKGRAPH_H

#include "track.h"

typedef struct {
    qint64  position;
    double  data;
} GraphData;

class TrackGraph : public Track {
    Q_OBJECT
public:
    TrackGraph(QObject* parent, QString _label, QString _title);
    ~TrackGraph();
    void paint(QPainter &painter);
    QString whatsthis() { return label; }
    virtual void calculateData() = 0;
    virtual void calculateData(QString& chr) = 0;
    virtual void clear();
    QMap<QString, int> size;
    QMap<QString, GraphData*> data;
protected:
    //virtual void paintRect(QPainter &painter, int x1, int h, int w);
    inline void paintRect(QPainter &painter, int x1, int x2, int h) {
        painter.drawRect(label_offset + x1, top + height - h, x2-x1, h - 1);
    }

 //   virtual inline void paintLine(QPainter &painter, int last_x, int h);
    inline void paintLine(QPainter &painter, int last_x, int h) {
        painter.drawLine(label_offset + last_x, top + height - h, label_offset + last_x, top + height);
    }
    virtual void paintLines(QPainter &painter) { Q_UNUSED(painter) }
    virtual void changeChromosome();
    virtual void changePosition();
    virtual void changeFilter();
    virtual void changeGroups();
    int window;
    Range paintedRange;
//    QList<int> paintedDataX;
//    QList<int> paintedDataH;
};

#endif // TRACKGRAPH_H
