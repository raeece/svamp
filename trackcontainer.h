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
#ifndef TRACKCONTAINER_H
#define TRACKCONTAINER_H

#include <QWidget>
#include <QCheckBox>
#include <QPaintEvent>
#include <QPainter>
#include <QPinchGesture>
#include "track.h"
#include "annotation.h"
#include "sequence.h"
#include "variation.h"

namespace Ui {
    class TrackContainer;
}

class TrackContainer : public QWidget {
    Q_OBJECT

public:
    explicit TrackContainer(QWidget *parent = 0);
    ~TrackContainer();

    virtual void setRange (QString chr, long from, long to);
    virtual void setQuality (long quality);
    virtual void setReadDepth (long readDepth);
    void changeChromosome();
    void changePosition();
    void changeFilter();
    void changeGroups();
    void recalculateLayout();
    void checkAndPaint();
    int posPerWidth();
    void trackenabledisable();
    Track* getTrack(QString name);
    Annotation* getAnnotation();
    Sequence* getSequence();
    Variation* getVariation();

    void startDrag (QPoint p);
    void moveDrag (QPoint p);
    void stopDrag ();
    bool dragging;

    Range range;
    int label_offset;

    QList <Track*> tracklist;
    QList <QCheckBox *> tracklistcheckbox;

    bool changed;
    bool delayPaint;
    QPoint drag_origin;

protected:
    virtual void changeEvent(QEvent *e);
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void mouseEvent(QMouseEvent *event,bool doublecklick=false);
    virtual void resizeEvent(QResizeEvent * event);
    virtual void wheelEvent(QWheelEvent * event);
    virtual bool gestureEvent(QGestureEvent *event);
    virtual bool event(QEvent * event);

private:
    Ui::TrackContainer *ui;
};

#endif // TRACKCONTAINER_H
