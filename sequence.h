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
#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <QObject>
#include <QMap>
#include "sourcesdialog.h"

class Sequence : public QObject {
    Q_OBJECT
public:
    explicit Sequence(QObject *parent = 0) : QObject(parent) { }
    ~Sequence() { clear(); }
    void clear();
    static void info (SourcesDialog* dialog, QString filename);
    bool loadFASTA (QString filename, QString filter);
    QString getCodon(QString chr, int position, int direction);
    QString reverse(QString codon);
    QChar reverse(QChar snp);
    bool checkChrom(QString chrom);

    QMap <QString,QString> chromosomes;
    QString source_file;

private:
    bool addSimpleSequence (QString name, QString sequence, QString filter);
    int seqRead;
};

#endif // SEQUENCE_H
