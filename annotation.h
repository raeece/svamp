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
#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QRect>
#include "sequence.h"
#include "sourcesdialog.h"

class AnnotationItem : public QObject {
    Q_OBJECT
public:
    AnnotationItem () { parent = NULL; }
    QString getDescription();
    QString name, type, chr, description;
    QChar strand;
    int from, to;
    QList<int> aminoAcids;
    AnnotationItem *parent;
    QRect display;
    QStringList alt_names;
};

class Annotation : public QObject {
    Q_OBJECT
public:
    explicit Annotation(QObject *parent = 0) : QObject(parent) { setupAminoAcids(); }
    ~Annotation() { clear(); }
    void clear();
    static void info (SourcesDialog* dialog, QString filename);
    bool loadGFF3 (QString filename, Sequence *seq, QString filter);

    QMap <QString,QList<AnnotationItem*> > items;
    QMap <QString,AnnotationItem* > names;
    QString source_file;
    Sequence *seq;

    void setupAminoAcids();
    QMap <long,int> getCodingMap(QString chr);
    int isCoding(QString chr, int pos);
    QString getAminoAcid(QString chr, int pos);
    QString getAminoAcid(QString codon, QString unrecog);
    int getAminoAcidCode(QString& codon1, int unrecog = -1);
    int getCodonPosition(QString& chr, int pos);
    bool synonymous(QString& codon1, QString& codon2);

    int getTotalRecord();

    QMap<QString,int> aminoAcids;
    QList<QString> codons;

};

#endif // ANNOTATION_H
