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
#include "variation.h"
#include "mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QString>
#include <QByteArray>
#include <QMessageBox>
#include <QStatusBar>
#include <QTextStream>
#include <QProgressDialog>

Variation::Variation(QObject *parent) : QObject(parent) {
    clear();
}

void Variation::clear() {

    foreach (QList<int> g, groups) g.clear();
    groups.clear();
    groupColor.clear();
    groups["Ungrouped"];
    groupColor.insert("Ungrouped",Qt::gray);
    phased = true;
    samples.clear();
    sample_r.clear();
    extrema.clear();
    foreach (QList<VariationPosition*> d, data) {
        foreach (VariationPosition* v, d) delete v;
        d.clear();
    }
}

void Variation::info(SourcesDialog *dialog, QString filename) {
    QString chromosome, lastChromosome = "";
    QMap<QString,int> count;
    QFile file(filename);
    if (! file.exists()) {
        QMessageBox msgBox;
        msgBox.setText("No variation file: " + filename);
        msgBox.exec();
        return;
    }
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    QString line = in.readLine();
    while (!line.isNull()) {
        if (line.startsWith("#")) {
        } else {
            chromosome = line.trimmed().split(QRegExp("\\s+"))[0];
            count[chromosome]++;
            if (0 == (count[chromosome] % 100)) dialog->update("variation", chromosome, count[chromosome]);
            if (lastChromosome != chromosome) dialog->update("variation", chromosome, count[chromosome]);
            lastChromosome = chromosome;
        }
        line = in.readLine();
    }
    dialog->update("variation", count);
    file.close();
}

bool Variation::loadVCF (QString filename, Sequence *seq, Annotation *ann, QString f) {
    qDebug() << "Variation::loading VCF file start";

    //int start = time(NULL);
    bool added = false;
    int linesRead = 0;

    source_file = filename;
    filter = f;
    QFile file(filename);
    if (! file.exists()) return false;
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream parse(&file);
    QString line = parse.readLine(), chr;
    QMap<QString,int> lines;
    MainWindow::instance->feedback.update(3,"Preparing for ");

    //reading VCF file from start to end just to calculate how many SNPs are there in order to reserve memory space
    while (!line.isNull()) {
        chr = line.simplified().section(' ',0,0);
        if (chr[0] != '#' && (filter == "" || chr == filter)) {
            lines[chr]++;
        }
        line = parse.readLine();
    }
    foreach (QString l, lines.keys()) {
        data[l].reserve(lines[l]);
    }

    parse.reset();
    file.close();
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    line = in.readLine();
    QString lastChrom="";
    while (!line.isNull()) {
        if (line.startsWith("#")) { // TODO : Parse this better!
            if (line.startsWith("#CHROM\t")) {
                QStringList parts = line.trimmed().split('\t');
                for (int a = 9; a < parts.size(); a++) {
                    groups["Ungrouped"].append(samples.size());

                    samples.append(parts[a]);
                }
            }
        } else {
            if (0 == (linesRead % 1000)) MainWindow::instance->feedback.update(3,QString::number(linesRead));
            linesRead++;
            QString cleanLine = line.replace(' ',',');
            QStringList parts = cleanLine.split(QRegExp("\\s+"));
            //qDebug() << "parts size here: " << parts.length() << "line: " <<cleanLine;
            if (filter != "" && parts[0] != filter) {
                line = in.readLine();
                continue;
            }

            int total = parts.length();
            int nSample = samples.length();
            int nField = nSample + 9;
            //if number of column in vcf is not correct
            if (nField != total)
            {
                qDebug() << "number of column in one (or more) line of VCF file is not correct";
                return false;
            }


            VariationPosition *vp = new VariationPosition();

            vp->chromosome = parts[0];
            if(number.isEmpty() || !number.contains(vp->chromosome)) //initialization
            {
                number[vp->chromosome].nSNP=0;
                number[vp->chromosome].nIndel=0;
                number[vp->chromosome].nMissing=0;
                number[vp->chromosome].nRef=0;
            }

            vp->position = parts[1].toInt();//will crash here if gff file is given in the vcf lineedit
            vp->id = parts[2];
            vp->genotypes.append(parts[3].toUpper()); //will crash here if db file is given instead of vcf

            vp->alt.insert(parts[3].toUpper(),0);

            if ("." != parts[4])
            {
                vp->genotypes.append(parts[4].toUpper().split(","));

                for(int j=0;j<vp->genotypes.size()-1;j++)
                {

                    vp->alt.insert(vp->genotypes.at(j+1),0);
                }
            }

            vp->quality = parts[5].toDouble();
            vp->filter = parts[6];
            vp->info = parts[7];
            if (0 > vp->quality) { vp->quality = 0; }

            foreach(QString gt, vp->genotypes) vp->genotypeLengths.append(gt.length());

            long readDepth = 1;
            QString info;
            foreach (info, parts[7].split(';')) {
                if (info.split("=")[0] == "DP") readDepth = info.split("=")[1].toLong();
            }

            QStringList format = parts[8].split(':');
            for (int a = 0; a < format.size(); a++) {
                vp->format[format[a]] = a;
            }

            if (! extrema.contains(vp->chromosome)) {
                extrema[vp->chromosome].qualityMin = 10000000000;
                extrema[vp->chromosome].qualityMax = 0;
                extrema[vp->chromosome].readDepthMin = 10000000000;
                extrema[vp->chromosome].readDepthMax = 0;
            }
            if (extrema[vp->chromosome].qualityMin > vp->quality) { extrema[vp->chromosome].qualityMin = vp->quality; }
            if (extrema[vp->chromosome].qualityMax < vp->quality) { extrema[vp->chromosome].qualityMax = vp->quality; }

            vp->data = new VariationData[parts.size() - 9];
            QRegExp re = QRegExp("[/|]");

            //find the index of each sub-field based on the format of column 9 in VCF file
            QMap<QString, int>::const_iterator itgt = vp->format.find("GT");
            QMap<QString, int>::const_iterator itdp = vp->format.find("DP");
            QMap<QString, int>::const_iterator itdpa = vp->format.find("DPA");
            QMap<QString, int>::const_iterator itdpc = vp->format.find("DPC");
            QMap<QString, int>::const_iterator itdpg = vp->format.find("DPG");
            QMap<QString, int>::const_iterator itdpt = vp->format.find("DPT");
            int alleles = 0, alleleCount = 0;
            int baseLength = vp->genotypeLengths[0];
            bool common =true;
            long h1,h2; // for chekcing if the variants are shared by all samples
            for (int a = 0; a < parts.size() - 9; a++) {
                QStringList data = parts[a + 9].split(':');

                //add checking if all the subfields is complete
                int nsubfield= data.size();

                if ("." != parts[a + 9] && itgt != vp->format.end() && itgt.value()<=nsubfield) {
                    QStringList gts = data[itgt.value()].split(re, QString::SkipEmptyParts);
                    vp->data[a].phased = data[itgt.value()].contains("|");

                    if(gts[0]==".")
                        vp->data[a].haplotype1 = -1;
                    else
                        vp->data[a].haplotype1 = gts[0].toLong();


                    //increment number of each allele for the allele frequency computation
                    vp->alt[vp->genotypes.at(gts[0].toLong())]= vp->alt[vp->genotypes.at(gts[0].toLong())]+1;

                    if(gts.count()>1)
                    {
                        if(gts[1]==".")
                            vp->data[a].haplotype2=-1;
                        else
                            vp->data[a].haplotype2 = gts[1].toLong();
                    }else
                        vp->data[a].haplotype2 = gts[0].toLong();

                    if(a==0)
                    {
                        h1=vp->data[a].haplotype1;
                        h2=vp->data[a].haplotype2;
                    }else if(common)
                    {
                        if(h1==vp->data[a].haplotype1 && h2==vp->data[a].haplotype2)// here h1 and h2's value are from previous iteration

                            common = true;
                        else
                            common = false;
                    }
                    h1=vp->data[a].haplotype1;
                    h2=vp->data[a].haplotype2;

                    //increment number of each allele for the allele frequency computation
                    if (gts.count() > 1)
                        vp->alt[vp->genotypes.at(gts[1].toLong())]= vp->alt[vp->genotypes.at(gts[1].toLong())]+1;
                    else
                        vp->alt[vp->genotypes.at(gts[0].toLong())]= vp->alt[vp->genotypes.at(gts[0].toLong())]+1;

                    /* Determining whether the variation is an indel :
                       If one of the haplotype is -1(missing), it will be determined by the other haplotype.
                       If both are missing, insertion and deletion are false
                      */
                    if(vp->data[a].haplotype1==-1 && vp->data[a].haplotype2!=-1)
                    {
                        vp->data[a].deletion=(vp->genotypeLengths[vp->data[a].haplotype2] < baseLength);
                        vp->data[a].insertion=(vp->genotypeLengths[vp->data[a].haplotype2] < baseLength);
                        vp->data[a].missing = true;
                        number[vp->chromosome].nMissing++;
                        if(vp->data[a].deletion) qDebug() << "position: " << vp->position << " haplo2 missing haplo1 a deletion";
                        if(vp->data[a].insertion) qDebug() << "position: " << vp->position << " haplo2 missing haplo1 an insertion";
                    }else if (vp->data[a].haplotype1!=-1 && vp->data[a].haplotype2==-1)
                    {
                        vp->data[a].deletion=(vp->genotypeLengths[vp->data[a].haplotype1] < baseLength);
                        vp->data[a].insertion=(vp->genotypeLengths[vp->data[a].haplotype1] < baseLength);
                        vp->data[a].missing = true;
                        number[vp->chromosome].nMissing++;
                        if(vp->data[a].deletion) qDebug() << "position: " << vp->position << " haplo1 missing haplo2 a deletion";
                        if(vp->data[a].insertion) qDebug() << "position: " << vp->position << " haplo1 missing haplo2 an insertion";
                    }else if (vp->data[a].haplotype1==-1 && vp->data[a].haplotype2==-1)
                    {
                        vp->data[a].deletion = false;
                        vp->data[a].insertion = false;
                        vp->data[a].missing = true;
                        number[vp->chromosome].nMissing++;
                    }else // if no haplotype is missing
                    {
                        vp->data[a].missing = false;
                        vp->data[a].deletion = (vp->genotypeLengths[vp->data[a].haplotype1] < baseLength) || (vp->genotypeLengths[vp->data[a].haplotype2] < baseLength);
                        vp->data[a].insertion = (vp->genotypeLengths[vp->data[a].haplotype1] > baseLength) || (vp->genotypeLengths[vp->data[a].haplotype2] > baseLength);
                    }

                    if (0 == vp->data[a].haplotype1) alleles++;
                    if (0 == vp->data[a].haplotype2) alleles++;
                    alleleCount += 2;
                } else {
                    vp->data[a].phased = true;
                    vp->data[a].haplotype1 = -1;
                    vp->data[a].haplotype2 = -1;
                    vp->data[a].deletion = false;
                    vp->data[a].insertion = false;
                    vp->data[a].missing = true;
                    number[vp->chromosome].nMissing++;
                }

                if(vp->data[a].haplotype1==0 && vp->data[a].haplotype2==0) // if the same with reference
                    number[vp->chromosome].nRef++;
                else if(vp->data[a].deletion || vp->data[a].insertion)
                    number[vp->chromosome].nIndel++;
                else if(!vp->data[a].missing)
                    number[vp->chromosome].nSNP++;

                // init here
                vp->data[a].coding = 0;
                // assign later more efficiently.

                vp->data[a].nonSynonymous = false;
                vp->data[a].synonymous = false;
                vp->data[a].readDepth = readDepth;
                phased = phased & vp->data[a].phased;

                //vp->data is a qstringlist of size n, which n is number of samples

                if ("." != parts[a + 9] && itdp != vp->format.end() && itdp.value()<nsubfield) vp->data[a].readDepth = data[itdp.value()].toLong();
                //  For the synonymous test set the strongest haplotype to the 'most' alternate.
                int genotype = vp->data[a].haplotype1 > vp->data[a].haplotype2 ? vp->data[a].haplotype1 : vp->data[a].haplotype2;
                if (genotype == -1) { genotype = 0; }
                vp->data[a].haplotypeStrong = vp->genotypes[genotype][0];
                if (vp->data[a].haplotype1 != vp->data[a].haplotype2) {
                    int dp1 = 0; int dp2 = 0;
                    if (itdpa.value()<=nsubfield && itdpa != vp->format.end() && vp->genotypes[vp->data[a].haplotype1] == "A") dp1 = data[itdpa.value()].toLong();
                    if (itdpc.value()<=nsubfield && itdpc != vp->format.end() && vp->genotypes[vp->data[a].haplotype1] == "C") dp1 = data[itdpc.value()].toLong();
                    if (itdpc.value()<=nsubfield && itdpg != vp->format.end() && vp->genotypes[vp->data[a].haplotype1] == "G") dp1 = data[itdpg.value()].toLong();
                    if (itdpt.value()<=nsubfield && itdpt != vp->format.end() && vp->genotypes[vp->data[a].haplotype1] == "T") dp1 = data[itdpt.value()].toLong();
                    if (itdpa.value()<=nsubfield && itdpa != vp->format.end() && vp->genotypes[vp->data[a].haplotype2] == "A") dp2 = data[itdpa.value()].toLong();
                    if (itdpc.value()<=nsubfield && itdpc != vp->format.end() && vp->genotypes[vp->data[a].haplotype2] == "C") dp2 = data[itdpc.value()].toLong();
                    if (itdpg.value()<=nsubfield && itdpg != vp->format.end() && vp->genotypes[vp->data[a].haplotype2] == "G") dp2 = data[itdpg.value()].toLong();
                    if (itdpt.value()<=nsubfield && itdpt != vp->format.end() && vp->genotypes[vp->data[a].haplotype2] == "T") dp2 = data[itdpt.value()].toLong();
                    if (dp1 > 0 && dp2 > 0) vp->data[a].haplotypeStrong = vp->genotypes[dp1 > dp2 ? vp->data[a].haplotype1 : vp->data[a].haplotype2][0];
                }
                if (extrema[vp->chromosome].readDepthMin > vp->data[a].readDepth && vp->data[a].readDepth != 0) {
                    extrema[vp->chromosome].readDepthMin = vp->data[a].readDepth;
                }
                if (extrema[vp->chromosome].readDepthMax < vp->data[a].readDepth) {
                    extrema[vp->chromosome].readDepthMax = vp->data[a].readDepth;
                }
            }
            vp->alleleFrequency = 1.0 * alleles / alleleCount;//frequency of the reference allele

            foreach(QString a, vp->alt.keys())
                vp->altFrequency[a] = vp->alt[a] * 1.0 / alleleCount * 100 ;

            vp->sharedByAll = common;

            data[vp->chromosome].append(vp);

//            if(vp->sharedByAll)
//                qDebug() << "Common SNP in " << vp->position;

            if (extrema[vp->chromosome].qualityMin == 10000000000) { extrema[vp->chromosome].qualityMin = 0; }
            if (extrema[vp->chromosome].readDepthMin == 10000000000) { extrema[vp->chromosome].readDepthMin = 0; }
            added = true;

            //check if chromosome field added has corresponding chrom in fasta loaded before
            if(lastChrom!=vp->chromosome || lastChrom=="")
            {
                lastChrom = vp->chromosome;
                bool found = seq->checkChrom(lastChrom);
                if(!found)
                {
                    QMessageBox warning(QMessageBox::Critical, "Chromosome Check", parts[0]+" is not found in sequence file. Please check if vcf has corresponding chromosome names.", QMessageBox::Ok);
                    warning.exec();
                    loadWell[parts[0]] = false;
                 }else
                    loadWell[parts[0]] = true;


            }

        }// end of for
        line = in.readLine();
    }
    file.close();
    MainWindow::instance->feedback.update(3,QString::number(linesRead));

    //  If there are no matching data points then let the user know a bad load has occured.
    if (! added) { return false; }

    //  Perform synonymous analysis.

    foreach (QString chr, data.keys()) {
        int len = data[chr].length();
         QMap<long,int> codingMap=ann->getCodingMap(chr);
        for (int i = 0; i < len; i++) {
            VariationPosition *vp = data[chr][i];
            int direction=codingMap[vp->position];
            if (0 == direction) continue;
            int codonPosition = ann->getCodonPosition(chr, vp->position);
            if (-1 == codonPosition) continue;
            QString codon = seq->getCodon(chr, codonPosition, direction);
            QString alternate;
            for (int j = 0; j < samples.count(); j++) {
                vp->data[j].coding=direction;
                if(vp->data[j].insertion || vp->data[j].deletion) continue;
                if(vp->data[j].haplotype1==0 && vp->data[j].haplotype2==0) continue;
                alternate = codon;
                if ( 1 == direction) alternate.replace(vp->position-codonPosition,1,vp->data[j].haplotypeStrong);
                vp->data[j].synonymous = ann->synonymous(codon, alternate);
                vp->data[j].nonSynonymous = ! vp->data[j].synonymous;
            }
        }
    }

    /*foreach(QString chr, number.keys())
    {
        qDebug() << chr << ": snp=" << number[chr].nSNP << " indel=" << number[chr].nIndel << " miss=" << number[chr].nMissing << " ref= " << number[chr].nRef ;
    }*/

    qDebug() << "Variation::loading VCF file end here";
    return true;
}

int Variation::getTotalVariants()
{
    int nVar=0;
    foreach(QString chr, data.keys())
    {
        nVar+= data[chr].size();
    }
    return nVar;
}

VariationPosition::~VariationPosition() {
    delete[] data;
    format.clear();
    genotypes.clear();
    genotypeLengths.clear();
}

QString VariationPosition::getDescription() {
    QString ret;
    if (!id.isEmpty()) ret += "<h2>" + id + "</h2>";
    if (data)
    ret += "<table width='100%' style='font-size:10pt'>";
    ret += "<tr><th align='left' width='50%'>Position</th><td width='50%'>" + chromosome + ":" + QString::number(position) + "</td></tr>";

    ret += "<tr><th align='left'>Reference</th><td>" + genotypes[0] + " ( "+QString::number(altFrequency[genotypes[0]]) +" % ) </td></tr>";
    if (genotypes.size() > 1)
    {
        ret += "<tr><th align='left'>Alternative(s)</th><td>";
        ret+= genotypes[1] + " ( "+ QString::number(altFrequency[genotypes[1]]) +" % )";
    for(int i=2;i<=genotypes.size()-1;i++)
        ret+= ", " + genotypes[i] + " ( "+ QString::number(altFrequency[genotypes[i]]) +" % ) ";
    }

    ret += "</td></tr><tr><th align='left'>Quality</th><td>" + QString::number(quality) + "</td></tr>";
    ret += "<tr><th align='left'>Filter</th><td>" + filter + "</td></tr>";
    ret += "<tr><th align='left'>Info</th><td>" + info + "</td></tr>";

    ret += "</table>";
    return ret;
}

QString VariationPosition::getGenotypeText (QString call) {
    QString ret;
    if (call == "0") ret = genotypes[0] + " (ref)";
    else if (call == ".") ret = "<i>missing</i>";
    else if (call.length() == 1) {
        ret = genotypes[call.toInt()] + " (nonref)";
    } else if (call.length() == 3) {
        QString p = call[1] == '|' ? "phased" : "unphased";
        ret = getGenotypeText(call.left(1)) + " / " + getGenotypeText (call.right(1)) + " [" + p + "]";
    }
    return ret;
}

bool Variation::loadCover(QString filename, QString chromName)
{
   coverage_file = filename;
   QFile file(filename);
    if (! file.exists()) {
        MainWindow::instance->statusBar()->clearMessage();
        return false;
    }
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    QString line = in.readLine();//read first line, which is name of chromosome
    if(line==chromName)
    {
        line = in.readLine();
        int sampleIndex=0;
        while (!line.isNull()) {
            QStringList pairs= line.split('\t');
            QString sampleName = pairs.at(0);
            QString info = pairs.at(1);
            QList<int> zeroPosList;
            QStringList valueList = info.split(':');
            foreach(QString token, valueList)
            {
                QStringList posList = token.split('-');
                if(posList.size()==2)
                {
                    for(int y=0;y<=posList.at(1).toInt()-posList.at(0).toInt();y++)
                    {
                        zeroPosList.append(posList.at(0).toInt()+y);
                    }
                }else
                {
                    zeroPosList.append(posList.at(0).toInt());
                }
            }
            zeroCov.insert(sampleIndex,zeroPosList);
            line = in.readLine();
            sampleIndex++;
        }
        MainWindow::instance->updateVariationTrack();
    }else
    {
        QMessageBox warning(QMessageBox::Critical, "File Error", "Corresponding chromosome name not found, please check coverage file", QMessageBox::Ok);
        warning.exec();
    }


    return true;
}
