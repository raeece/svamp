/*
 *  SVAMP - Sequence iation Analysis , Map, and Phylogeny
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
#include "annotation.h"
#include "mainwindow.h"
#include <QFile>
#include <QByteArray>
#include <QMessageBox>
#include <QStatusBar>
#include <QTextStream>
#include <QProgressDialog>
#include <QUrl>
#include <QDebug>

void Annotation::clear() {
    foreach(QList<AnnotationItem*> i, items) {
        foreach(AnnotationItem* a, i)
            delete a;
        i.clear();
    }
    items.clear();
}

void Annotation::info (SourcesDialog* dialog, QString filename) {
    QString chromosome, lastChromosome = "";
    QMap<QString,int> count;
    QFile file(filename);
    if (! file.exists()) {
        QMessageBox msgBox;
        msgBox.setText("No annotation file: " + filename);
        msgBox.exec();
        return;
    }
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    QString line = in.readLine();
    while (!line.isNull()) {
        QStringList parts = line.split('\t');
        line = in.readLine();
        if (parts.size() < 9) continue; // Hmmm...
        if (parts[0].startsWith("##")) continue; // Ignore header (I know I shouldn't...)
        if (parts[2].toLower() == "supercontig") continue;  //  Ignore contigs...
        chromosome = parts[0].split('|').last();
        count[chromosome]++;
        if (0 == (count[chromosome] % 100)) dialog->update("annotation", chromosome, count[chromosome]);
        if (lastChromosome != chromosome) dialog->update("annotation", chromosome, count[chromosome]);
        lastChromosome = chromosome;
    }
    dialog->update("annotation", count);
    file.close();
}

bool Annotation::loadGFF3 (QString filename, Sequence *sequence, QString filter) {
    //int start = time(NULL);
    int linesRead = 0;
    source_file = filename;
    seq = sequence;
    QFile file(filename);
    if (! file.exists()) {
        MainWindow::instance->statusBar()->clearMessage();
        return false;
    }
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    QString line = in.readLine();
    AnnotationItem *last_gene = NULL;
    QString lastChr="";
    while (!line.isNull()) {
        QStringList parts = line.split('\t');
        line = in.readLine();
        if (0 == (linesRead % 1000)) MainWindow::instance->feedback.update(2,QString::number(linesRead));
        if (parts[0].startsWith("##")) continue; // Ignore header (I know I shouldn't...)
        if (parts.size() < 9) continue; // Hmmm...

        AnnotationItem *item;
        try {
            item = new AnnotationItem(); // dangling ?
        } catch (...) {
            MainWindow::instance->outOfMemory(QString("AnnotationItem"));
        }
        item->type = parts[2].toLower();
        //item->chr = parts[0].split('|').last();

        item->chr = parts[0];

        //check if chromosome field added has corresponding chrom in gff/fasta loaded before
        if (lastChr != item->chr)
        {
            bool found = sequence->checkChrom(item->chr);
            if(!found)
            {
                QMessageBox warning(QMessageBox::Critical, "Chromosome Check", item->chr+" is not found in sequence file. Please check if gff has same chromosome names.", QMessageBox::Ok);
                warning.exec();
            }
            lastChr = item->chr;
        }

        if (item->type == "supercontig" || (filter != "" && item->chr != filter)) { // Bah!
            delete item;
            qDebug() << "item deleted";
            continue;
        }
        linesRead++;
        if (item->type.startsWith("repeat")) { item->type = "repeat"; }
        item->from = parts[3].toInt();
        item->to = parts[4].toInt();
        item->strand = parts[6][0];
        if (item->type == "gene") {
            last_gene = item;
//            int codon = item->from;
//            while (codon < item->to) {
//                item->aminoAcids.append(getAminoAcidCode(seq->getCodon(item->chr, codon)));
//                codon += 3;
//            }
        } else if (last_gene && last_gene->from<=item->from && last_gene->to>=item->to && last_gene->chr==item->chr) {
            item->parent = last_gene;
        }

        QStringList ids = parts[8].split(';');
        for (int a = 0; a < ids.size(); a++) {
            QStringList l = ids[a].split('=');
            if (l.size() != 2) continue;
            QString key = l[0].toLower();
            if (key == "id") {
                item->name = l[1].split('|').last();
                names[item->name] = item;
            } else if (key == "description") {
                QUrl u;
                u.setEncodedPath(l[1].toAscii());
                item->description = u.toString();
//                item->description = l[1];
                item->description.replace("+"," ");
            } else if (key == "dbxref" || key == "alias" || key == "sanger") {
                names[l[1]] = item;
                QStringList sl = (key+":"+l[1]).split(",");
                for (int b = 0; b < sl.size(); b++) {
                    item->alt_names.append(sl[b]);
                }
            }
        }
        items[item->chr].append(item);
    }
    file.close();
    MainWindow::instance->feedback.update(2,QString::number(linesRead));
    ///qDebug() << "Annotation::loadGFF" << linesRead << items.count();

    //int end=time(NULL);
    //qDebug() << "GFF loading took " << end-start << " seconds";

    return true;
}

QString AnnotationItem::getDescription() {
    QString html;
    html += "<h2>" + name + "</h2>";
    html += "<b>" + type + "</b><br/>";
    html += chr + " : " + QString::number(from) + "&ndash;" + QString::number(to) + " [" + strand + "]<br/>";
    if (!alt_names.empty()) {
//        dbxref:ApiDB_PlasmoDB:MAL13P1.380
        // http://plasmodb.org/plasmo/showRecord.do?name=GeneRecordClasses.GeneRecordClass&project_id=PlasmoDB&source_id=MAL13P1.380
        html += "<ul>";
        for (int a = 0; a < alt_names.size(); a++) {
            QString s = alt_names[a];
            if (s.startsWith("dbxref:ApiDB_PlasmoDB:")) {
                QStringList l = s.split(':');
                s = "<a href='http://plasmodb.org/plasmo/showRecord.do?name=GeneRecordClasses.GeneRecordClass&project_id=PlasmoDB&source_id="+l[2]+"'>"+s+"</a>";
            }
            html += "<li>" + s + "</li>";
        }
        html += "</ul>";
//        html += "<ul><li>" + alt_names.join("</li><li>") + "</li></ul>";
    }
    html += description;
    if (parent) html += "<hr/><i>Part of:</i><br/>" + parent->getDescription();
    return html;
}

int Annotation::isCoding(QString chr, int pos) {
    for (int i = 0; i < items[chr].length(); i++) {
        AnnotationItem *ai = items[chr][i];
        if (pos < ai->from) continue;
        if (pos > ai->to) continue;
        if (ai->type == "cds" && ai->strand == '+') return +1;
        if (ai->type == "cds" && ai->strand == '-') return -1;
    }
    return 0;
}

QMap <long,int> Annotation::getCodingMap(QString chr){
     QMap <long,int> codingMap;
     QList <AnnotationItem*> aiptrs = items[chr];
     for (int i = 0; i < aiptrs.length(); i++) {
         AnnotationItem *ai = aiptrs[i];
         if (ai->type == "cds"){
             int strand=0;
             if(ai->strand == '+') {
                strand=+1;
             }
             if(ai->strand == '-') {
                strand=-1;
             }
             for (long l=ai->from;l<ai->to;l++){
                 codingMap[l]=strand;
             }
         }
      }
     return codingMap;
}
int Annotation::getCodonPosition(QString& chr, int pos) {
    for (int i = 0; i < items[chr].length(); i++) {
        AnnotationItem *ai = items[chr][i];
        if (pos < ai->from) continue;
        if (pos > ai->to) continue;
        if (ai->type != "cds")  continue;
        return pos - (pos - ai->from) % 3;
    }
    return -1;
}

QString Annotation::getAminoAcid(QString chr, int pos) {
    return pos == getCodonPosition(chr,pos) ? getAminoAcid(seq->getCodon(chr, pos, isCoding(chr, pos)), "XXX") : "XXX";
}

QString Annotation::getAminoAcid(QString codon, QString unrecog) {
    return aminoAcids.contains(codon) ? codons[aminoAcids[codon]] : unrecog;
}

int Annotation::getAminoAcidCode(QString& codon, int unrecog) {
    return aminoAcids.contains(codon) ? aminoAcids[codon] : unrecog;
}

bool Annotation::synonymous(QString& codon1, QString& codon2) {
    return getAminoAcidCode(codon1, -1) == getAminoAcidCode(codon2, -2);
}

int Annotation::getTotalRecord()
{
    int n=0;
    foreach(QString chr, items.keys())
    {
        n+=items[chr].size();
    }
    return n;
}

void Annotation::setupAminoAcids() {
    aminoAcids["TTT"] = 0;  // phe
    aminoAcids["TTC"] = 0;
    aminoAcids["TTA"] = 1;  // leu
    aminoAcids["TTG"] = 1;
    aminoAcids["TCT"] = 2;  // ser
    aminoAcids["TCC"] = 2;
    aminoAcids["TCA"] = 2;
    aminoAcids["TCG"] = 2;
    aminoAcids["TAT"] = 3;  // tyr
    aminoAcids["TAC"] = 3;
    aminoAcids["TAA"] = 4;  // STOP
    aminoAcids["TAG"] = 4;
    aminoAcids["TGT"] = 5;  // cys
    aminoAcids["TGC"] = 5;
    aminoAcids["TGA"] = 4;  // STOP
    aminoAcids["TGG"] = 6;  // trp

    aminoAcids["CTT"] = 1;  // leu
    aminoAcids["CTC"] = 1;
    aminoAcids["CTA"] = 1;
    aminoAcids["CTG"] = 1;
    aminoAcids["CCT"] = 7;  // pro
    aminoAcids["CCC"] = 7;
    aminoAcids["CCA"] = 7;
    aminoAcids["CCG"] = 7;
    aminoAcids["CAT"] = 8;  // his
    aminoAcids["CAC"] = 8;
    aminoAcids["CAA"] = 9;  // gln
    aminoAcids["CAG"] = 9;
    aminoAcids["CGT"] = 10;  // arg
    aminoAcids["CGC"] = 10;
    aminoAcids["CGA"] = 10;
    aminoAcids["CGG"] = 10;

    aminoAcids["ATT"] = 11;  // ile
    aminoAcids["ATC"] = 11;
    aminoAcids["ATA"] = 11;
    aminoAcids["ATG"] = 12;  // met
    aminoAcids["ACT"] = 13;  // thr
    aminoAcids["ACC"] = 13;
    aminoAcids["ACA"] = 13;
    aminoAcids["ACG"] = 13;
    aminoAcids["AAT"] = 14;  // asn
    aminoAcids["AAC"] = 14;
    aminoAcids["AAA"] = 15;  // lys
    aminoAcids["AAG"] = 15;
    aminoAcids["AGT"] = 2;   // ser
    aminoAcids["AGC"] = 2;
    aminoAcids["AGA"] = 10;  // arg
    aminoAcids["AGG"] = 10;

    aminoAcids["GTT"] = 16;  // val
    aminoAcids["GTC"] = 16;
    aminoAcids["GTA"] = 16;
    aminoAcids["GTG"] = 16;
    aminoAcids["GCT"] = 17;  // ala
    aminoAcids["GCC"] = 17;
    aminoAcids["GCA"] = 17;
    aminoAcids["GCG"] = 17;
    aminoAcids["GAT"] = 18;  // asp
    aminoAcids["GAC"] = 18;
    aminoAcids["GAA"] = 19;  // glu
    aminoAcids["GAG"] = 19;
    aminoAcids["GGT"] = 20;  // gly
    aminoAcids["GGC"] = 20;
    aminoAcids["GGA"] = 20;
    aminoAcids["GGG"] = 20;

    codons.insert(0, "phe");
    codons.insert(1, "leu");
    codons.insert(2, "ser");
    codons.insert(3, "tyr");
    codons.insert(4, "STOP");
    codons.insert(5, "cys");
    codons.insert(6, "trp");
    codons.insert(7, "pro");
    codons.insert(8, "his");
    codons.insert(9, "gln");
    codons.insert(10, "arg");
    codons.insert(11, "ile");
    codons.insert(12, "met");
    codons.insert(13, "thr");
    codons.insert(14, "asn");
    codons.insert(15, "lys");
    codons.insert(16, "val");
    codons.insert(17, "ala");
    codons.insert(18, "asp");
    codons.insert(19, "glu");
    codons.insert(20, "gly");
}
