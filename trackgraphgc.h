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
#ifndef TRACKGRAPHGC_H
#define TRACKGRAPHGC_H

#include "trackgraph.h"

class TrackGraphGC : public TrackGraph {
    Q_OBJECT
public:
    TrackGraphGC(QObject* parent) : TrackGraph(parent, "GC density", "GCDensity") { }
    virtual void calculateData();
    virtual void calculateData(QString& chr);
    virtual void paint(QPainter &painter);
    virtual void paintLines(QPainter &painter);
    virtual void changeChromosome() {}
    virtual void changePosition() {}
    virtual void changeFilter() {}
    virtual void changeGroups() {}
};

#endif // TRACKGRAPHGC_H
