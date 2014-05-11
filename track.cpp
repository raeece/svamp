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
#include "track.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

Track::Track(QObject *parent) : QObject(parent) {
    height = 0;
    label_offset = 0;
    forceRedraw = false;
    if (parent) {
        area = ((MainWindow*)parent)->getUI()->tracks;
        area->changed = true;
    } else {
        area = NULL;
    }
    range.chromosome =  "XXX";
    range.from = 1;
    range.to = 1;
    range.readDepth = 1;
    range.quality = 0;
    range.gene = true;
    range.insertion = true;
    range.deletion = true;
    range.synonymous = true;
    range.nonSynonymous = true;
    title = "none";
}

Track::~Track() {
//    qDebug() << "Track::~Track " << label;
    clear();
}

void Track::clear() {
//    qDebug() << "Track::clear " << label;
}

void Track::paint(QPainter &painter) {
    Q_UNUSED(painter)
}

void Track::paintLabel(QPainter &painter) {
    if (label_offset <= 0) return;
    if (label.isEmpty()) return;

    QRect r (2,top,label_offset-5,height-1);

    painter.setPen(Qt::black);
    painter.setBrush(Qt::NoBrush);
    painter.setFont(QFont("Arial", 8));
    painter.drawText(r, Qt::AlignCenter, label);
    painter.setPen(Qt::lightGray);
    painter.drawRoundedRect(r,5,5);
}

void Track::setRange (Range _range) {
    range = _range;
}

void Track::recalculateLayout() {
}

void Track::mouseEvent(QMouseEvent *event,QString region,bool doubleclick) {
    Q_UNUSED(event)
    Q_UNUSED(region)
    Q_UNUSED(doubleclick)
    MainWindow::instance->showProps("");
}

void Track::doTheDrag(QMouseEvent *event) {
    if (Qt::LeftButton == event->button()) {
        area->startDrag (event->pos());
    } else if (Qt::NoButton == event->button() && (Qt::LeftButton && event->buttons())) {
        area->moveDrag (event->pos());
    } else if (area->dragging && event->buttons() == Qt::NoButton) {
        area->stopDrag();
    }
}
