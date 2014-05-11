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
#include "trackcontainer.h"
#include "ui_trackcontainer.h"
#include "mainwindow.h"
#include <QDebug>
#include <QMessageBox>

TrackContainer::TrackContainer(QWidget *parent) : QWidget(parent), ui(new Ui::TrackContainer) {
    dragging = false;
    changed = false;
    delayPaint = false;
    label_offset = 0;
    range.chromosome = "";
    range.from = 1;
    range.to = 1;
    range.quality = 0;
    range.readDepth = 1;
    ui->setupUi(this);
    setMouseTracking(true);
    setAttribute(Qt::WA_AcceptTouchEvents);
    grabGesture(Qt::PinchGesture);
}

TrackContainer::~TrackContainer() {
    for (int a = 0; a < tracklist.size(); a++) {
        delete tracklist[a];
    }
    delete ui;
}

void TrackContainer::mouseMoveEvent(QMouseEvent *event) {
    mouseEvent (event);
}

void TrackContainer::mousePressEvent(QMouseEvent *event) {
    mouseEvent (event);
}

void TrackContainer::mouseDoubleClickEvent(QMouseEvent *event) {
    mouseEvent (event, true);
}

void TrackContainer::mouseEvent(QMouseEvent *event,bool doublecklick) {
//    qDebug() << "TrackContainer::mouseEvent";
    for (int a = 0; a < tracklist.size(); a++) {
        if (tracklist[a]->rLabel.contains(event->pos())) tracklist[a]->mouseEvent(event,"label",doublecklick);
        else if (tracklist[a]->rContent.contains(event->pos())) tracklist[a]->mouseEvent(event,"content",doublecklick);
        else continue;
        return; // Can't have overlapping reginons, no?
    }
    MainWindow::instance->showProps("");
}

void TrackContainer::resizeEvent (QResizeEvent * event) {
//    qDebug() << "TrackContainer::resizeEvent " << event->type();
    Q_UNUSED(event)
    changed = true;
    this->update();
}

void TrackContainer::checkAndPaint() {
    if (delayPaint) { return; }
    if (changed) recalculateLayout();
    update();
}

void TrackContainer::paintEvent(QPaintEvent *event) {
//    qDebug() << "TrackContainer::paintEvent " << event->type();
    Q_UNUSED(event)
    if (delayPaint) return;
//    if (tracklist.size() > 0 && height() != tracklist.last()->top + tracklist.last()->height) setMinimumHeight(tracklist.last()->top + tracklist.last()->height);
    QPainter painter(this);
    painter.fillRect(0,0,width(),height(),QColor(255,255,255));
    for (int a = 0; a < tracklist.size(); a++) {
        tracklist[a]->rLabel = QRect(0, tracklist[a]->top, tracklist[a]->label_offset, tracklist[a]->height);
        painter.setClipRect(tracklist[a]->rLabel);
        QCheckBox *trackcheckbox = tracklistcheckbox[a];
        trackcheckbox->setGeometry(2,tracklist[a]->top+1,10,10);
        tracklist[a]->paintLabel(painter);
        tracklist[a]->rContent = QRect(tracklist[a]->label_offset, tracklist[a]->top, tracklist[a]->width, tracklist[a]->height);
        painter.setClipRect(tracklist[a]->rContent);
        if(trackcheckbox->isChecked()){
        tracklist[a]->paint(painter);
        }
    }
}


void TrackContainer::trackenabledisable()
{
    //qDebug() << "TrackContainer::trackenabledisable";
        update();
}

void TrackContainer::changeEvent(QEvent *e) {
//    qDebug() << "TrackContainer::changeEvent";
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void TrackContainer::setRange (QString chr, long from, long to) {
//    qDebug() << "TrackContainer::setRange " << chr << " " << from << " " << to;
    range.chromosome = chr;
    range.from = from;
    range.to = to;
    for (int a = 0; a < tracklist.size(); a++) {
        if (tracklist[a]->range.chromosome == range.chromosome && tracklist[a]->range.from == range.from && tracklist[a]->range.to == range.to) continue;
        tracklist[a]->setRange(range);
        changed = true;
    }
}

void TrackContainer::setQuality (long quality) {
//    qDebug() << "TrackContainer::setQuality";
    range.quality = quality;
    for (int a = 0; a < tracklist.size(); a++) {
        if (tracklist[a]->range.quality == range.quality) continue;
        tracklist[a]->setRange(range);
        changed = true;
    }
}

void TrackContainer::setReadDepth (long readDepth) {
//    qDebug() << "TrackContainer::setReadDepth";
    range.readDepth = readDepth;
    for (int a = 0; a < tracklist.size(); a++) {
        if (tracklist[a]->range.readDepth == range.readDepth) continue;
        tracklist[a]->setRange(range);
        changed = true;
    }
}

void TrackContainer::changeChromosome() {
//    qDebug() << "TrackContainer::changeChromosome";
    for (int a = 0; a < tracklist.size(); a++) {
        tracklist[a]->changeChromosome();
    }
}

void TrackContainer::changePosition() {
//    qDebug() << "TrackContainer::changePosition";
    for (int a = 0; a < tracklist.size(); a++) {
        tracklist[a]->changePosition();
    }
}

void TrackContainer::changeFilter() {
//    qDebug() << "TrackContainer::changeFilter";
    for (int a = 0; a < tracklist.size(); a++) {
        tracklist[a]->changeFilter();
    }
}

void TrackContainer::changeGroups() {
//    qDebug() << "TrackContainer::changeGroups";
    for (int a = 0; a < tracklist.size(); a++) {
        tracklist[a]->changeGroups();
    }
}

void TrackContainer::recalculateLayout() {
//    qDebug() << "TrackContainer::recalculateLayout";
    int y = 1;
    label_offset = 0;
    for (int a = 0; a < tracklist.size(); a++) {
        tracklist[a]->top = y;
        tracklist[a]->width = width();
        tracklist[a]->recalculateLayout();
        y += tracklist[a]->height + 2;
        if (tracklist[a]->label_offset > label_offset) label_offset = tracklist[a]->label_offset;
    }
    for (int a = 0; a < tracklist.size(); a++) {
        tracklist[a]->label_offset = label_offset;
        tracklist[a]->width -= label_offset;
    }
    if (tracklist.size()) {
        int h = tracklist.last()->top + tracklist.last()->height;
        if (height() != h) setMinimumHeight(h);
    }
    changed = false;
}

int TrackContainer::posPerWidth() {
    return (this->width() - label_offset) / 12; // GUESSING FIXME
}

Track* TrackContainer::getTrack(QString name) {
    for (int a = 0; a < tracklist.size(); a++) {
        if (tracklist[a]->whatsthis() != name) continue;
        return tracklist[a];
    }
    return NULL;
}

Annotation *TrackContainer::getAnnotation () {
    Track *track = getTrack("annotation");
    if (track) return &((TrackAnnotation*)track)->data;
    else return NULL;
}

Sequence *TrackContainer::getSequence () {
    Track *track = getTrack("sequence");
    if (track) return &((TrackSequence*)track)->seq;
    else return NULL;
}

Variation *TrackContainer::getVariation () {
    Track *track = getTrack("variation");
    if (track) return &((TrackVariation*)track)->variants;
    else return NULL;
}

void TrackContainer::wheelEvent (QWheelEvent * event) {
    return; // Not ready
    int numDegrees = event->delta() / 8;
    qreal numSteps = numDegrees / 15;
    if (event->orientation() == Qt::Horizontal) {
        qreal factor = (qreal) (range.to - range.from) / 5;
        numSteps *= factor; // Relative to zoom factor
        int f = range.from - numSteps;
        int t = range.to - numSteps;
        while (f < 1) {
            f++;
            t++;
        }
        MainWindow::instance->setRange(range.chromosome,f,t);
        event->accept();
    } else { // Vertical
        // Ignore - might be used by the actual widget...
    }
}

bool TrackContainer::gestureEvent (QGestureEvent *event) {
    event->accept(Qt::PinchGesture);
    QPinchGesture *g = (QPinchGesture*) event->gesture(Qt::PinchGesture);
    if (g) {
//        QMessageBox::information(NULL,QString("Still here"),"!");

        qreal f = g->lastScaleFactor() - g->scaleFactor();
        int ow = range.to - range.from;
        int nw = ow + f * ow;
        int diff = (ow - nw) / 2;
        int from = range.from + diff;
        if (from < 1) from = 1;
        MainWindow::instance->setRange(range.chromosome,from,from+nw);
    }
    return true;
}

bool TrackContainer::event (QEvent * event) {
    if (event->type() == QEvent::Gesture) {
        QGestureEvent *g = (QGestureEvent*) event;
        return gestureEvent (g);
        //QMessageBox::information(NULL,QString("Still here"),"!!!");
    }
/*
    if (event->type() == QEvent::TouchBegin) {
        ((QTouchEvent*)event)->accept();
        return true;
    }
    if (event->type() == QEvent::TouchBegin) {
        QMessageBox::information(NULL,QString("Still here"),"!!!");
    }
*/
    return QWidget::event(event);
}

void TrackContainer::startDrag (QPoint p) {
    drag_origin = p;
    setCursor (Qt::ClosedHandCursor);
    grabMouse ();
    dragging = true;
}

void TrackContainer::moveDrag (QPoint p) {
    long d = p.x() - drag_origin.x();
    long w = range.to - range.from;
    long diff = d * w / (width() - label_offset);
    MainWindow::instance->setRange(range.chromosome, range.from - diff, range.to - diff);
    drag_origin = p;
}

void TrackContainer::stopDrag() {
    releaseMouse();
    setCursor(Qt::ArrowCursor);
    dragging = false;
}
