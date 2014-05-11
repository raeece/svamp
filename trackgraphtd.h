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
#ifndef TRACKGRAPHTD_H
#define TRACKGRAPHTD_H

#include "trackgraph.h"

class TrackGraphTD : public TrackGraph {
    Q_OBJECT
public:
    TrackGraphTD(QObject* parent) : TrackGraph(parent, "Tajima's D\nBarriers: -2, +2", "TajimasD") { }
    virtual void calculateData();
    virtual void calculateData(QString& chr);
protected:
    //virtual void paintRect(QPainter &painter, int x1, int h, int w);
    inline virtual void paintRect(QPainter &painter, int x1, int x2, int h) {
        if (h > 0)
            painter.drawRect(label_offset + x1, top + height / 2 - h / 8, x2-x1,  h / 8 - 1);
        else
            painter.drawRect(label_offset + x1, top + height / 2 - h / 8, x2-x1, -h / 8 - 1);
    }

   // inline virtual void paintLine(QPainter &painter, int last_x, int h);
    inline void paintLine(QPainter &painter, int last_x, int h) {
        painter.drawLine(label_offset + last_x, top + height / 2 - h / 5, label_offset + last_x, top + height / 2);
    }
    virtual void paintLines(QPainter &painter);
};

#endif // TRACKGRAPHTD_H
