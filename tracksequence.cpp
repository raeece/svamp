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
#include "tracksequence.h"
#include <QDebug>
#include <QMessageBox>
#include "trackcontainer.h"
#include "annotation.h"

TrackSequence::TrackSequence(QObject *parent) : Track(parent) {
    main_font_size = 10;
    num_font_size = 10;
    label = "Sequence";
    title = label;
}

void TrackSequence::paint(QPainter &painter) {
//    qDebug() << "TrackSequence::paint start " << range.from << " " << range.to;
    Annotation *ann = area->getAnnotation();

    if (!seq.chromosomes.contains(range.chromosome)) return;

    long w = range.to - range.from + 1;

    painter.setPen(Qt::black);
    double s = 1.0 * width / w;
    if (s < 5.0) {
        long steps = 10;
        long sf = 5;
        while (1) {
            long cnt = 0;
            for (long pos = steps * (range.from / steps); cnt < 20 && pos <= range.to; pos += steps) cnt++;
            if (cnt < 20) break;
            steps *= sf;
            sf = (sf == 5) ? 2 : 5;
        }
        painter.setFont(QFont("Arial", num_font_size));
        for (long pos = steps * (range.from / steps); pos <= range.to; pos += steps) {
            long x = (pos-range.from) * s;
            if (x < 0) continue;
            QRect r(label_offset + x - 50, top + 12, (int) s-1 + 100, height);
            painter.drawText(r, Qt::AlignTop|Qt::AlignHCenter, QString::number(pos));
            long y1 = top + 2;
            long y2 = top + 10;
            x = (r.left()+r.right())/2;
            painter.drawLine(x,y1,x,y2);
        }
    } else {
        for (long pos = range.from; pos <= range.to; pos++) {
            QRect r(label_offset + (pos-range.from)*width/w, top, width/w-1, height);
            if (r.left() < label_offset) continue;
            painter.drawText(r, Qt::AlignTop|Qt::AlignHCenter, QString(seq.chromosomes[range.chromosome][(int)pos]));
            QString aminoAcid = ann->getAminoAcid(range.chromosome, pos - 1);
            if ("XXX" != aminoAcid) {
                int codonWidth = (pos-range.from+2)*width/w - (pos-range.from-1)*width/w;
                QRect s(label_offset + (pos-range.from-1)*width/w, top + 2*height/3 - 2, codonWidth, height/3 + 1);
                painter.drawText(s, Qt::AlignBottom|Qt::AlignHCenter, aminoAcid);
                painter.drawRect(s);
            }
            if (pos == 1 || pos % 10 == 0) {
                if (main_font_size != num_font_size) painter.setFont(QFont("Arial", num_font_size));
                r = QRect ((int) (label_offset + (pos-range.from)*s - 50), top + 12, (int) s-1 + 100, height);
                if (r.left() < label_offset) continue;
                painter.drawText(r, Qt::AlignTop|Qt::AlignHCenter, QString::number(pos));
                if (main_font_size != num_font_size) painter.setFont(QFont("Arial", main_font_size));
            }
        }
    }

    painter.setPen(Qt::lightGray);
    painter.drawRoundedRect(QRect(label_offset,top,width-1,height-1),5,5);

//    qDebug() << "TrackSequence::paint end";
}

void TrackSequence::recalculateLayout() {
//    qDebug() << "TrackSequence::recalculateLayout";

    height = (main_font_size + num_font_size) + 20;
    label_offset = 100;
}

void TrackSequence::mouseEvent(QMouseEvent *event,QString region,bool doubleclick) {
//    qDebug() << "TrackSequence::mouseEvent";
    Q_UNUSED(doubleclick)
    if (region == "content") {
        doTheDrag(event);
    }
}
