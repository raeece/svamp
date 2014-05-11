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
#include "trackannotation.h"
#include "mainwindow.h"
#include <QDebug>
#include <QList>
#include <QRect>
#include <QMessageBox>

TrackAnnotation::TrackAnnotation(QObject *parent) : Track(parent) {
    label = "Annotation";
    title = label;
}

void TrackAnnotation::paint(QPainter &painter) {
//    qDebug() << "TrackAnnotation::paint";
    //qDebug() << "range.chromosome: " << range.chromosome;
    if (!data.items.contains(range.chromosome)) return;

    QList <AnnotationItem *> items;
    //qDebug() << "nGene: " << data.items[range.chromosome].size();
    for (int i = 0; i < data.items[range.chromosome].size(); i++) {
        if (data.items[range.chromosome][i]->from > range.to) continue;
        if (data.items[range.chromosome][i]->to < range.from) continue;
        items.append(data.items[range.chromosome][i]);
    }

    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 9));
    long w = range.to - range.from + 1;
    double s = 1.0 * width / w;
    for (int i = 0; i < items.size(); i++) {
        long f = (items[i]->from - range.from) * s;
        long t = (items[i]->to - range.from + 1) * s;
        if (f < 0) f = 0;
        if (t > width) t = width;
        long yoff = items[i]->parent ? 20 : 0;
        long h = 15;
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::NoBrush);
        bool show_text = false;
        if (items[i]->type == "gene") { painter.setPen(Qt::black); show_text = true; }
        else if (items[i]->type == "repeat") { h = 6; painter.setBrush(Qt::blue); }
        else if (items[i]->type == "cds") { yoff += 8; h = 6; painter.setBrush(Qt::red); }
        // else if (items[i]->type == "exon") { h = 6; painter.setBrush(Qt::blue); }
        else if (items[i]->type == "exon") { continue; }
        else if (items[i]->type == "mrna") { continue; } // Don't draw
        else { yoff += 8; painter.setBrush(Qt::yellow); show_text = true; } // Everything else goes there...

        QRect r (label_offset + f, top + 2 + yoff, t-f, h);
        items[i]->display = r;
        painter.drawRoundedRect(r,5,5);
        if (show_text) {
            painter.setPen(Qt::black);
            r.setTop(r.top()+2);
            painter.drawText(r, Qt::AlignCenter, items[i]->name);
        }
        if (r.width() >= 20 && (items[i]->type=="repeat" || items[i]->type=="cds")) {
            QString s = items[i]->strand == '+' ? QString::fromUtf8("→") : QString::fromUtf8("←");
            painter.setPen(Qt::white);
            painter.setFont(QFont("Arial", 11));
            painter.drawText(r, Qt::AlignCenter, s);
            painter.setFont(QFont("Arial", 9));
        }
    }

    painter.setPen(Qt::lightGray);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(QRect(label_offset,top,width-1,height-1),5,5);
}

void TrackAnnotation::recalculateLayout() {
//    qDebug() << "TrackAnnotation::recalculateLayout";
    height = 45;
    label_offset = 50;
}

void TrackAnnotation::runSearch (QString query, QList <SearchResult*> &results) {
//    qDebug() << "TrackAnnotation::runSearch";

    QMap <QString,AnnotationItem*>::iterator i = data.names.find(query);
    QString query_l = query.toLower();

    if (i == data.names.end()) {
        for (i = data.names.begin(); i != data.names.end(); i++) {
            if (i.key().toLower() == query_l) break;
        }
    }

    if (i == data.names.end()) {
        bool found = false;
        for (i = data.names.begin(); i != data.names.end(); i++) {
            if (i.key().indexOf(query,0,Qt::CaseInsensitive) != -1) {
                SearchResult *sr = new SearchResult;
                sr->fromAnnotationItem (this, i.value());
                results.append(sr);
                found = true;
            }
        }
        if (found) return;
    }

    if (i != data.names.end()) {
        SearchResult *sr = new SearchResult;
        sr->fromAnnotationItem (this, i.value());
        results.append(sr);
        return;
    }

    for (QMap <QString,QList <AnnotationItem*> >::iterator j = data.items.begin(); j != data.items.end(); j++) {
        for (int k = 0; k < j.value().size(); k++) {
            if (j.value()[k]->description.indexOf(query,0,Qt::CaseInsensitive) != -1) {
                SearchResult *sr = new SearchResult;
                sr->fromAnnotationItem (this, j.value()[k]);
                results.append(sr);
            }
        }
    }
}

void TrackAnnotation::mouseEvent(QMouseEvent *event,QString region,bool doubleclick) {
//    qDebug() << "TrackAnnotation::mouseEvent start";
    if (Qt::LeftButton == event->button()) MainWindow::instance->stickyProps(false);
    if (region == "content") {
        for (int i = 0; i < data.items[range.chromosome].size(); i++) {
            if (data.items[range.chromosome][i]->from > range.to) continue;
            if (data.items[range.chromosome][i]->to < range.from) continue;
            if (!data.items[range.chromosome][i]->display.contains(event->pos())) continue;
            MainWindow::instance->showProps(data.items[range.chromosome][i]->getDescription());
            if (doubleclick) {
                long padding = (data.items[range.chromosome][i]->to - data.items[range.chromosome][i]->from) / 20;
                MainWindow::instance->setRange(range.chromosome,data.items[range.chromosome][i]->from,data.items[range.chromosome][i]->to,padding);
                MainWindow::instance->stickyProps(false);
            } else if (Qt::LeftButton == event->button()) {
                MainWindow::instance->stickyProps(true);
            }
            return;
        }
        doTheDrag(event);
    }
    MainWindow::instance->showProps("");
//    qDebug() << "TrackAnnotation::mouseEvent end";
}
