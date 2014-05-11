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
#ifndef TRACKVARIATION_H
#define TRACKVARIATION_H

#include "track.h"
#include "variation.h"

class TrackVariation : public Track {
    Q_OBJECT
public:
    explicit TrackVariation(QObject *parent = 0);
    ~TrackVariation();
    virtual void clear();

    virtual void paintLabel(QPainter &painter);
    virtual void paint(QPainter &painter);
    virtual void recalculateLayout();
    virtual QString whatsthis() { return "variation"; }
    virtual void mouseEvent(QMouseEvent *event,QString region,bool doubleclick);

    Variation variants;
    long sample_height;
    int last_paint_start, last_paint_end;

protected:
    bool paintArea (VariationPosition *v, QPainter &painter, long x1, long y, long x2, int f, int t, const QColor &color);
    bool paintArea (QPainter &painter, long x1, long y, long x2, int f, int t, const QColor &color);

};

#endif // TRACKVARIATION_H
