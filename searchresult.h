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
#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H

#include <QObject>

class Track;
class AnnotationItem;

class SearchResult : public QObject
{
    Q_OBJECT
public:
    explicit SearchResult(QObject *parent = 0);

    QString displayname;
    Track *track;

    long from, to;
    QString chr;

    void fromAnnotationItem (Track *_track, AnnotationItem *ai);

    AnnotationItem *annotation_item;

signals:

public slots:

};

#endif // SEARCHRESULT_H
