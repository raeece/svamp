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
#include "mainwindow.h"
#include "trackgraph.h"
#include "trackgraphgc.h"
#include "trackgraphvd.h"
#include "trackgraphfst.h"
#include "trackgraphtd.h"
#include "trackcontainer.h"
#include "variation.h"
#include <cmath>
#include <QDebug>
#include <QMessageBox>

TrackGraph::TrackGraph(QObject *parent, QString _label, QString _title) : Track(parent) {
    ///qDebug() << "TrackGraph::TrackGraph";
    label = _label;
    title = _title;
    height = 45;
    label_offset = 100;
    window = 2;
}

TrackGraph::~TrackGraph() {
//    qDebug() << "TrackGraph::~TrackGraph " << label;
    clear();
}

void TrackGraph::clear() {
//    qDebug() << "TrackGraph::clear " << label;
    foreach (QString chr, data.keys()) {
        if (data.contains(chr) && data[chr]) delete[] data[chr];
    }
    data.clear();
    size.clear();
}

void TrackGraph::paint(QPainter &painter) {
//    qDebug() << "TrackGraph::paint start " << label << " " << range.chromosome << " " << size[range.chromosome];
    long w = range.to - range.from + 1;
    double scale = 1.0 * width / w;
    painter.setPen(Qt::darkBlue);
    painter.setBrush(Qt::darkBlue);

//    qDebug() << scale;

    if (! size.contains(range.chromosome) || ! size[range.chromosome]) return;
    if (! data.contains(range.chromosome) || ! data[range.chromosome]) return;

//    if (scale < 1.0 && paintedRange.chromosome == range.chromosome && paintedRange.from == range.from && paintedRange.to == range.to && false == forceRedraw) {
//        for (long i = 0; i < paintedDataX.count(); i++) {
//            paintLine(painter, paintedDataX[i], paintedDataH[i]);
//        }
//    } else {
        forceRedraw = false;
        paintedRange = range;
//        paintedDataX.clear();
//        paintedDataH.clear();

//        long last_x = 0;
        double last_dmin = 0, last_dmax = 0;

        GraphData *dptr=data[range.chromosome];
        for (long i = 0; i < size[range.chromosome]; i++) {
            GraphData d = dptr[i];
            //GraphData d = data[range.chromosome][i];
            if (d.position > range.to) break; // Assuming VCF file was sorted by position...
            if (d.position < range.from) continue;
            if (d.data == 0.0) continue;

            long x1 = scale * (d.position - range.from);
            long x2 = scale * (d.position - range.from + 1) - 1;
            if (last_dmin > d.data){
                last_dmin = d.data;
            } else if (last_dmax < d.data){
                last_dmax = d.data;
            }
            if (x1 < x2) {
                int h = d.data * height;
                paintRect(painter, x1, x2, h);
//                last_x = x1;
                last_dmin = 0;
                last_dmax = 0;
//            } else if (x1 != last_x) {
            } else {
                int h = last_dmax > -last_dmin ? last_dmax * height : last_dmin * height;
                paintLine(painter, x1, h);
//                paintedDataX.append(last_x);
//                paintedDataH.append(h);
//                last_x = x1;
                last_dmin = 0;
                last_dmax = 0;
            }
        }
//    }

    painter.setPen(Qt::lightGray);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(QRect(label_offset,top,width-1,height-1),5,5);

    paintLines(painter);

//    qDebug() << "TrackGraph::paint end";
}

/*void TrackGraph::paintRect(QPainter &painter, int x1, int x2, int h) {
    painter.drawRect(label_offset + x1, top + height - h, x2-x1, h - 1);
}
*/

/*inline void TrackGraph::paintLine(QPainter &painter, int last_x, int h) {
    painter.drawLine(label_offset + last_x, top + height - h, label_offset + last_x, top + height);
}
*/
/*void TrackGraphTD::paintRect(QPainter &painter, int x1, int x2, int h) {
    if (h > 0)
        painter.drawRect(label_offset + x1, top + height / 2 - h / 8, x2-x1,  h / 8 - 1);
    else
        painter.drawRect(label_offset + x1, top + height / 2 - h / 8, x2-x1, -h / 8 - 1);
}
*/

/*inline void TrackGraphTD::paintLine(QPainter &painter, int last_x, int h) {
    painter.drawLine(label_offset + last_x, top + height / 2 - h / 5, label_offset + last_x, top + height / 2);
}
*/
void TrackGraphGC::paint(QPainter &painter) {
//    qDebug() << "TrackGraph::paint start " << label << " " << range.chromosome << " " << size[range.chromosome];


    long w = range.to - range.from + 1;
    double scale = 1.0 * width / w;
    painter.setPen(Qt::darkBlue);
    painter.setBrush(Qt::darkBlue);

//    qDebug() << scale;

    if (! size.contains(range.chromosome) || ! size[range.chromosome]) return;
    if (! data.contains(range.chromosome) || ! data[range.chromosome]) return;

//    if (scale < 1.0 && paintedRange.chromosome == range.chromosome && paintedRange.from == range.from && paintedRange.to == range.to && false == forceRedraw) {
//        for (long i = 0; i < paintedDataX.count(); i++) {
//            paintLine(painter, paintedDataX[i], paintedDataH[i]);
//        }
//    } else {
        forceRedraw = false;
        paintedRange = range;
//        paintedDataX.clear();
//        paintedDataH.clear();

//        long last_x = 0;
        double last_dmin = 0, last_dmax = 0;

        QPolygon qp(w);

        GraphData *dptr=data[range.chromosome];
        for (long i = range.from-1; i < range.to; i++) {
            GraphData d = dptr[i];
            if (d.data == 0.0) continue;
            long x1 = scale * (d.position - range.from);
            long x2 = scale * (d.position - range.from + 1) - 1;
            if (last_dmin > d.data){
                last_dmin = d.data;
            } else if (last_dmax < d.data){
                last_dmax = d.data;
            }
            if (x1 < x2) {
                int h = d.data * height;
                qp<<QPoint(label_offset+x1,top+height-h);
                qp<<QPoint(label_offset+x2,top+height-h);
                last_dmin = 0;
                last_dmax = 0;
            } else {
                int h = last_dmax > -last_dmin ? last_dmax * height : last_dmin * height;
                qp<<QPoint(label_offset+x1,top+height-h);
                last_dmin = 0;
                last_dmax = 0;
            }
        }

    painter.drawPolyline(qp);
    painter.setPen(Qt::lightGray);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(QRect(label_offset,top,width-1,height-1),5,5);

    paintLines(painter);

//    qDebug() << "TrackGraph::paint end";
}

void TrackGraphGC::paintLines(QPainter &painter) {
    painter.setPen(Qt::red);
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(label_offset + 1, top + height / 2, label_offset + width - 1, top + height / 2);
}

void TrackGraphTD::paintLines(QPainter &painter) {
    painter.setBrush(Qt::NoBrush);
    painter.setPen(Qt::red);
    painter.drawLine(label_offset + 1, top +     height / 4, label_offset + width - 1, top +     height / 4);
    painter.drawLine(label_offset + 1, top + 3 * height / 4, label_offset + width - 1, top + 3 * height / 4);
    painter.setPen(Qt::darkBlue);
    painter.drawLine(label_offset + 1, top + height / 2, label_offset + width - 1, top + height / 2);
}

//  Calculate the graph data and encourage a repaint...
void TrackGraph::changeChromosome() {
//    qDebug() << "TrackGraph::changeChromosome";
    calculateData(range.chromosome);
    forceRedraw = true;
}

void TrackGraph::changePosition() {
//    qDebug() << "TrackGraph::changePosition";
    calculateData(range.chromosome);
    forceRedraw = true;
}

void TrackGraph::changeFilter() {
//    qDebug() << "TrackGraph::changeFilter";
    calculateData(range.chromosome);
    forceRedraw = true;
}

void TrackGraph::changeGroups() {
//    qDebug() << "TrackGraph::changeGroups";
    calculateData(range.chromosome);
    forceRedraw = true;
}

void TrackGraphGC::calculateData() {
//    qDebug() << "TrackGraphGC::calculateData start";
    Sequence *seq = area->getSequence();
    if (!seq) return;
    foreach (QString chr, seq->chromosomes.keys()) {
        calculateData(chr);
    }
//    qDebug() << "TrackGraphGC::calculateData end";
}

void TrackGraphGC::calculateData(QString& chr) {
//    qDebug() << "TrackGraphGC::calculateData start " << chr;
    //Sequence *seq = area->getSequence();
    QString sequence = area->getSequence()->chromosomes[chr];
    qint64 l = sequence.length();
    GraphData *d = NULL;
    try {
        d = new GraphData[l];
    } catch(...) {
        MainWindow::instance->outOfMemory("TrackGraphGC");
    }
    int count = 0;
    for (int i = 0; i < 24; i++) {
        if (sequence[i] == 'G' || sequence[i] == 'C') { count ++; }
    }
    for (int i = 0; i < l; i++) {
        if (i + 25 < l && (sequence[i+25] == 'G' || sequence[i+25] == 'C')) { count++; }
        if (i > 25     && (sequence[i-25] == 'G' || sequence[i-25] == 'C')) { count--; }
        d[i].position = i;
        d[i].data = count / 51.0;
    }
    size[chr] = l;
    data[chr] = d;
//    qDebug() << "TrackGraphGC::calculateData end";
}

void TrackGraphVD::calculateData() {
//    qDebug() << "TrackGraphVD::calculateData start";
    Sequence *seq = area->getSequence();
    Variation *var = area->getVariation();
    if (!seq) return;
    if (!var) return;
    foreach (QString chr, seq->chromosomes.keys()) {
        calculateData(chr);
    }
//    qDebug() << "TrackGraphVD::calculateData end";
}

void TrackGraphVD::calculateData(QString& chr) {
//    qDebug() << "TrackGraphVD::calculateData start " << chr;
    Variation *var = area->getVariation();
    Sequence *seq = area->getSequence();
    if (!var) return;
    if (!seq) return;
    QString sequence = seq->chromosomes[chr];
    int len = sequence.length();
    if (0 == len) return;
    if (! data[chr]) {
        try {
            data[chr] = new GraphData[len];
        } catch (...) {
            MainWindow::instance->outOfMemory("TrackGraphVD");
        }
    }
    size[chr] = len;
    for (int i = 0; i < len; i++) {
        data[chr][i].position = i;
        data[chr][i].data = 0;
    }
    for (int i = 0; i < var->data[chr].size(); i++) {
        VariationPosition *v = var->data[chr][i];
        if (v->quality < range.quality) continue;
        for (int j = 0; j < var->samples.size(); j++) {
            VariationData gt = v->data[j];
            if (gt.readDepth < range.readDepth) continue;
            if (gt.coding == 0 && ! MainWindow::instance->isNonCoding()) continue;
            if (gt.deletion && ! MainWindow::instance->isIndel()) continue;
            if (gt.insertion && ! MainWindow::instance->isIndel()) continue;
            if (gt.synonymous && ! MainWindow::instance->isSynonymous()) continue;
            if (gt.nonSynonymous && ! MainWindow::instance->isNonSynonymous()) continue;
            if (v->sharedByAll && !MainWindow::instance->isCommon()) continue;
            for (int k = 0; k < 2*window+1; k++) {
                if (v->position - window + k >= 0 && v->position - window + k < len) {
                    data[chr][v->position - window + k].data++;
                }
            }
            break;
        }
    }
    int maxdata = 0;
    for (int i = 0; i < len; i++) if (data[chr][i].data > maxdata) maxdata = data[chr][i].data;
    if (maxdata > 0) for (int i = 0; i < len; i++) data[chr][i].data /= 1.0 * maxdata;
//    qDebug() << "TrackGraphVD::calculateData end";
}

void TrackGraphFST::calculateData() {
//    qDebug() << "TrackGraphFST::calculateData start";
    Sequence *seq = area->getSequence();
    Variation *var = area->getVariation();
    if (!seq) return;
    if (!var) return;
    int r = var->groups.count();
    if (r <= 1) return;

    foreach (QString chr, seq->chromosomes.keys()) {
        calculateData(chr);
    }
//    qDebug() << "TrackGraphFST::calculateData end";
}

void TrackGraphFST::calculateData(QString& chr) {
    //qDebug() << "TrackGraphFST::calculateData start" << chr;
    Variation *var = area->getVariation();
    if (!var) return;

    QList<VariationPosition*> vp = var->data[chr];
    qint64 len = vp.length();
    if (0 == len) return;
    GraphData *d = NULL;
    try {
        d = (data[chr] ? data[chr] : new GraphData[len]);
    } catch (...) {
        MainWindow::instance->outOfMemory("TrackGraphFST");
    }

    for (int i = 0; i < len; i++) {
        d[i].position = vp[i]->position;
        d[i].data = 0;
    }
    size[chr] = len;
    data[chr] = d;

    //  Crude number of groups filter.
    int groups = 0;
    foreach (QString gp, var->groups.keys()) if (MainWindow::instance->isGroup(gp)) groups++;
    if (groups <= 1) return;

    for (int i = 0; i < len; i++) {
        VariationPosition *v = vp[i];
        if (v->position > range.to) break; // Assuming VCF file was sorted by position...
        if (v->position < range.from) continue;
        if (v->quality < range.quality) continue;

        double fst = 0.0;
        int nbar = 0;

        // Have a stronger groups filter.
        groups = 0;
        QList<int> alleleList, heteroList, sampleList;
        foreach (QString gp, var->groups.keys()) {
            if (! MainWindow::instance->isGroup(gp)) continue;
            int alleles = 0, samples = 0, hetero = 0, alleleCount = 0;;
            for (long gpi = 0; gpi < var->groups[gp].size(); gpi++) {
                VariationData gt = vp[i]->data[var->groups[gp][gpi]];
                if (gt.readDepth < range.readDepth) continue;
                if (gt.coding == 0 && ! MainWindow::instance->isNonCoding()) continue;
                if (gt.deletion && ! MainWindow::instance->isIndel()) continue;
                if (gt.insertion && ! MainWindow::instance->isIndel()) continue;
                if (gt.synonymous && ! MainWindow::instance->isSynonymous()) continue;
                if (gt.nonSynonymous && ! MainWindow::instance->isNonSynonymous()) continue;
                if (v->sharedByAll && !MainWindow::instance->isCommon()) continue;

                if (0 == gt.haplotype1) alleles++;
                if (0 == gt.haplotype2) alleles++;
                if (0 == gt.haplotype1 && 0 != gt.haplotype2) hetero++;
                if (0 != gt.haplotype1 && 0 == gt.haplotype2) hetero++;
                if (-1 != gt.haplotype1)
                {
                    samples++;
                    alleleCount++;
                }
            }
            if (0 < alleleCount) {
                fst += alleleCount * pow(vp[i]->alleleFrequency - 0.5 * alleles / alleleCount, 2);
                nbar += alleleCount;

            }
            //  Have a minimum number of samples to include for this group.
            if (10 <= samples) {
                alleleList.append(alleles);
                heteroList.append(hetero);
                sampleList.append(samples);
                groups++;
            }
        }
        //  Estimating F-Statistics for the Analysis of Population Structure, Weir and Cockerham, Evolution 1984
        d[i].position = vp[i]->position;
        if (0 < vp[i]->alleleFrequency && vp[i]->alleleFrequency < 1 && groups > 1) {
            double r = groups, n = 0, n2 = 0, nbar = 0, hbar = 0, pbar = 0, nc = 0;
            for (int j = 0; j < r; j++) {
                n += sampleList[j];
                n2 += sampleList[j] * sampleList[j];
                pbar += alleleList[j];
                hbar += heteroList[j];
//                qDebug() << j << sampleList[j] << alleleList[j] << heteroList[j];
            }
            nbar = n / r;
            hbar /=   n;
            pbar /= 2*n;
            nc = (n - 1.0 * n2 / n) / (r - 1);
            double s2 = 0;
            for (int j = 0; j < r; j++) {
                s2 += sampleList[j] * pow(0.5 * alleleList[j] / sampleList[j] - pbar, 2.0);
            }
            s2 /= (r - 1) * nbar;
            double a, b, c;
            a = nbar / nc *   (s2 - (pbar*(1-pbar) - (r-1)/r*s2 - hbar/4) / (nbar-1) );
            b = nbar / (nbar - 1) * (pbar*(1-pbar) - (r-1)/r*s2 - (2*nbar - 1)*hbar/4/nbar);
            c = hbar / 2;
//            qDebug() << hbar << pbar << nbar <<  n << n2 << nc << s2;
//            qDebug() << a << b << c << (a / (a+b+c));
            d[i].data = a / (a + b + c);
            if (nbar>0) //for allele frequency view in the description box
                 d[i].data = groups * fst / (groups - 1) / vp[i]->alleleFrequency / (1 - vp[i]->alleleFrequency) / nbar;
        } else {
            d[i].data = 0;
        }
    }
    size[chr] = len;
    data[chr] = d;
    //qDebug() << "TrackGraphFST::calculateData end";
}

void TrackGraphTD::calculateData() {
//    qDebug() << "TrackGraphTD::calculateData start";
    Sequence *seq = area->getSequence();
    Variation *var = area->getVariation();
    if (!seq) return;
    if (!var) return;
    int r = var->groups.count();
    if (r <= 1) return;

    foreach (QString chr, seq->chromosomes.keys()) {
        calculateData(chr);
    }
//    qDebug() << "TrackGraphTD::calculateData end";
}

void TrackGraphTD::calculateData(QString& chr) {
//    qDebug() << "TrackGraphTD::calculateData start" << chr;
    Variation *var = area->getVariation();
    if (!var) return;

    QList<VariationPosition*> vp = var->data[chr];
    qint64 len = vp.length();
    if (0 == len) return;
    GraphData *d = NULL;
    try {
        d = (data[chr] ? data[chr] : new GraphData[len]);
    } catch (...) {
        MainWindow::instance->outOfMemory("TrackGraphTD");
    }
    for (int i = 0; i < len; i++) {
        d[i].position = vp[i]->position;
        d[i].data = 0.0;
    }
    size[chr] = len;
    data[chr] = d;

    int samples = var->samples.count();
    if (samples <= 1) return;

    //  Precalculate some variance variables.
    double a1 = 0.0, a2 = 0.0, b, c, e1, e2;
    for (int i = 1; i < samples; i++) {
        a1 += 1.0 / i;
        a2 += 1.0 / i / i;
    }
    b = 1.0 * (samples + 1) / 3 / (samples - 1);
    c = b - 1 / a1;
    e1 = c / a1;
    b = 2.0 * (samples*samples + samples + 3) / 9 / samples / (samples - 1);
    c = b - 1.0 * (samples + 2) / samples / a1 + a2 / a1 / a1;
    e2 = c / (a1*a1 + a2);

    int liWindowLength = 2*window+1;
    if (var->phased) {
        liWindowLength *= 2;
    }
    int *genotype = new int[liWindowLength];
    for (int i = 0; i < liWindowLength; i++) genotype[i] = 0;

    for (int i = 0; i < len; i++) {
        VariationPosition *v = vp[i];
        if (v->position > range.to + window) break; // Assuming VCF file was sorted by position...
        if (v->position < range.from - window) continue;
        if (v->quality < range.quality) continue;
        int offset = i % (2*window+1);
        genotype[offset] = 0;
        for (int j = 0; j < samples; j++) {
            VariationData gt = vp[i]->data[j];
            if (gt.readDepth < range.readDepth) continue;
            if (gt.coding == 0 && ! MainWindow::instance->isNonCoding()) continue;
            if (gt.deletion && ! MainWindow::instance->isIndel()) continue;
            if (gt.insertion && ! MainWindow::instance->isIndel()) continue;
            if (gt.synonymous && ! MainWindow::instance->isSynonymous()) continue;
            if (gt.nonSynonymous && ! MainWindow::instance->isNonSynonymous()) continue;
            if (v->sharedByAll && !MainWindow::instance->isCommon()) continue;

            //  Assume that 0 is the reference.
            if (! var->phased) {
                if ((vp[i]->data[j].haplotype1 == -1 || vp[i]->data[j].haplotype1 == 0) &&
                    (vp[i]->data[j].haplotype2 == -1 || vp[i]->data[j].haplotype2 == 0)) {
                    genotype[offset]++;
                }
            } else {
                if ((vp[i]->data[j].haplotype1 == -1 || vp[i]->data[j].haplotype1 == 0)) {
                    genotype[2 * offset]++;
                }
                if ((vp[i]->data[j].haplotype2 == -1 || vp[i]->data[j].haplotype2 == 0)) {
                    genotype[2 * offset + 1]++;
                }
            }
        }
        int poly = 0;
        double td = 0.0;
        for (int j = 0; j < liWindowLength; j++) {
            poly += (0 == genotype[j] || genotype[j] == samples) ? 0 : 1;
            td += (samples - genotype[j]) * genotype[j];
        }
        if (poly > 0) {
            td /= 0.5 * (samples - 1) * samples;
            td -= 1.0 * poly / a1;
            td /= sqrt(e1*poly + e2*poly*(poly-1));
        } else {
            td = 0.0;
        }
        d[i].position = vp[i]->position;
        d[i].data = td;
//        qDebug() << i << d[i].data;
    }

    delete[] genotype;
//    qDebug() << "TrackGraphTD::calculateData end";
}

