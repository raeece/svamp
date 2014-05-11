#include "snpselectiondialog.h"
#include "ui_snpselectiondialog.h"
#include "mainwindow.h"
#include "treeoptionsdialog.h"

SNPSelectionDialog::SNPSelectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SNPSelectionDialog)
{
    ui->setupUi(this);

    //Variation * var = MainWindow::instance->getUI()->tracks->getVariation();
    //QStringList chromList = var->data.keys();
    Sequence * seq = MainWindow::instance->getUI()->tracks->getSequence();
    QList <QString> chrlist;
    chrlist = seq->chromosomes.keys();
    ui->listWidget_chroms->clear();
    ui->listWidget_chroms->addItems(chrlist);

    ui->listWidget_chroms->setSelectionMode(QAbstractItemView::ExtendedSelection);
    //make the background of textedit_format transparent
    //ui->textEdit_format->viewport()->setAutoFillBackground(false);

    ui->label_depth->setText(MainWindow::instance->getUI()->lineDepth->text());
    ui->label_quality->setText(MainWindow::instance->getUI()->lineQuality->text());
}

SNPSelectionDialog::~SNPSelectionDialog()
{
    delete ui;
}


void SNPSelectionDialog::changeButtonText(){
    ui->pushButton_save->setText("Save");
}

void SNPSelectionDialog::on_pushButton_save_clicked()
{

  Variation * var = MainWindow::instance->getUI()->tracks->getVariation();
  QString userText, currentChrom ;
  QStringList chromList;
  QFile * bedfile;
  long from ,to;
  QList<VariationPosition * > vpList;

  if(ui->radiob_cview->isChecked())
  {
      currentChrom = MainWindow::instance->getUI()->chrBox->currentText();
      from = MainWindow::instance->getUI()->tracks->range.from;
      to = MainWindow::instance->getUI()->tracks->range.to;
  }else if(ui->radiob_chrom->isChecked())
  {
      foreach(QListWidgetItem * item, ui->listWidget_chroms->selectedItems())
          chromList.append(item->text());
  }else if(ui->radiob_custom->isChecked())
  {
      userText =   ui->plainTextEdit_selectedsnps->toPlainText();
      currentChrom = MainWindow::instance->getUI()->chrBox->currentText();
  }else if(ui->radioButton_bed->isChecked())
  {
      bedfile = new QFile(ui->lineEdit_bedfile->text());
  }

  vpList = prepareSnpList(var, currentChrom, chromList, from, to, userText, bedfile);

  vpList = filterSnps(vpList, ui->checkBox_noncoding->isChecked(), ui->checkBox_depth->isChecked() , ui->checkBox_qual->isChecked());

  //set infoText
  MainWindow::instance->infoText = toInfoText(currentChrom, chromList, from, to, userText, bedfile);

  if(vpList.empty())
  {
      if(ui->radiob_custom->isChecked())
      {
          QMessageBox warning(QMessageBox::Critical, "No SNP", "No variation found in selected region. Please check format of custom text", QMessageBox::Ok);
          warning.exec();
          return;
      }
      else{
          QMessageBox warning(QMessageBox::Critical, "No SNP", "No variation found in selected region.", QMessageBox::Ok);
          warning.exec();
          return;
      }
  }else
  {
      // set pushbutton to disabled to avoid multiple click
      ui->pushButton_save->setEnabled(false);

      bool indels = ui->checkBox_indels->isChecked();
      if(pcoa)
      {
          MainWindow::instance->runPcoa(areAllSamples,vpList, indels);
      }else
      {
          if(savemode){
              emit startSaveAlignment(vpList,indels,selectedsamples);
          }
          else{
              emit startTreeComputation(vpList, indels,selectedsamples);
          }
      }
  }
  close();
}

void SNPSelectionDialog::on_radiob_chrom_toggled(bool checked)
{
    ui->listWidget_chroms->setEnabled(checked);
    if(checked)
        ui->listWidget_chroms->setCurrentItem(ui->listWidget_chroms->item(0));
}

void SNPSelectionDialog::on_radiob_custom_toggled(bool checked)
{
    //ui->plainTextEdit_selectedsnps->clear();
    ui->plainTextEdit_selectedsnps->setEnabled(checked);
}

void SNPSelectionDialog::on_radiob_cview_toggled(bool checked)
{
 ui->plainTextEdit_selectedsnps->clear();
 ui->plainTextEdit_selectedsnps->setEnabled(!checked);

 ui->listWidget_chroms->setEnabled(!checked);
}


void SNPSelectionDialog::on_lineEdit_bedfile_selectionChanged()
{
    QString f = QFileDialog::getOpenFileName(this, tr("Select BED format file"),ui->lineEdit_bedfile->text(), tr("BED file (*)"));
    if (!f.isNull() && f != ui->lineEdit_bedfile->text()) {
        ui->lineEdit_bedfile->setText(f);
    }
}

void SNPSelectionDialog::on_radioButton_bed_toggled(bool checked)
{
    ui->lineEdit_bedfile->setEnabled(checked);
    if(checked)
    {
        QString f = QFileDialog::getOpenFileName(this, tr("Select BED format file"),ui->lineEdit_bedfile->text(), tr("BED file (*)"));
        if (!f.isNull() && f != ui->lineEdit_bedfile->text()) {
            ui->lineEdit_bedfile->setText(f);
        }
    }
}

QList<VariationPosition *> SNPSelectionDialog::getInputVar(Variation *var, QString chromosome, long from, long to)
{
    QList<VariationPosition *> vpList;
    QList<VariationPosition *> storedVarList = var->data[chromosome];
    int nSnps = var->data[chromosome].size();
    for (int i=0; i<nSnps ; i++)//iterating through the SNPs
    {
        VariationPosition *v = storedVarList[i];
        if (v->position < from ) continue;
        if (v->position >= from && v->position <= to)
        {
            vpList.append(v);
        }
        if(v->position > to ) break;

    }
    return vpList;
}


QList<VariationPosition *> SNPSelectionDialog::getInputVar(Variation *var, QStringList chromosome)
{
     QList<VariationPosition *> vpList;
     foreach(QString chrom, chromosome)
     {
         QList<VariationPosition*> vpChromList = var->data[chrom];
         vpList.append(vpChromList);
     }
     return vpList;
}

QList<VariationPosition *> SNPSelectionDialog::getInputVar(Variation *var, QString chromosome, QString userText)
{
    QList<VariationPosition *> vpList;

    QStringList lines = userText.split("\n");
    foreach(QString line, lines)
    {
        QStringList numberList;
        if(line.contains(":"))
        {
            QStringList chromvar = line.split(":");
            chromosome = chromvar.at(0);
            qDebug() <<"chrom "<< chromosome;
            numberList = chromvar.at(1).split(",");
        }else
        {
            numberList = line.split(",");
        }

        QList<VariationPosition*> returnedList;
        foreach(QString number, numberList)
        {
            if(number.contains("-"))
            {
                QStringList nList = number.split("-");
                long x1 = nList.at(0).toLong();
                long x2 = nList.at(1).toLong();
                returnedList.append(getVarPosBetween(var, chromosome, x1,x2));

            }else
            {
                long snppos = number.toLong();
                returnedList.append(getVarPosBetween(var, chromosome,snppos,snppos));//checking whether the input snp is correct
            }
        }
        vpList.append(returnedList);
    }
    return vpList;
}

QList<VariationPosition *> SNPSelectionDialog::getInputVar(Variation *var, QFile * bedfile )
{
    QList<VariationPosition *> vpList;

    if (! bedfile->exists()) {
        QMessageBox msgBox;
        msgBox.setText("BED file can't be found");
        msgBox.exec();
        return vpList;
    }
    bedfile->open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream bed(bedfile);

    QString line = bed.readLine();
    while(!bed.atEnd())
    {
        QStringList tokens = line.split("\t");
        vpList.append(getVarPosBetween(var, tokens.at(0),tokens.at(1).toLong(),tokens.at(2).toLong()));

        line=bed.readLine();
    }

    foreach(VariationPosition * vp , vpList)
        qDebug() << vp->position;
    return vpList;
}

QList<VariationPosition *> SNPSelectionDialog::getVarPosBetween(Variation * var, QString chromosome, long x1, long x2)
{
        QList<VariationPosition *> varList;
        if(x2<x1) return varList;

        int nSnps = var->data[chromosome].size();
        QList<VariationPosition*> vpList = var->data[chromosome]; //stored variants

        for (int i=0; i< nSnps; i++)//iterating through the SNPs
        {
            VariationPosition *v = vpList[i];

            if (v->position <x1) continue; // Assuming VCF file was sorted by position...

            if (v->position >= x1 && v->position <= x2)
            {
               varList.append(v);
            }

            if(v->position>x2) break;
        }
        return varList;
}

QList<VariationPosition *> SNPSelectionDialog::prepareSnpList(Variation *var, QString currentChrom, QStringList chromosomes, long from, long to, QString userText, QFile *bedfile)
{
    QList<VariationPosition *> vpList;

    if(ui->radiob_cview->isChecked())// snps in current view
    {
         vpList = getInputVar(var,currentChrom, from,to );
    }
    else if(ui->radiob_chrom->isChecked())
    {
        vpList = getInputVar(var, chromosomes);
    }
    else  if(ui->radiob_custom->isChecked())
    {
        vpList = getInputVar(var, currentChrom,userText);
    }else if(ui->radioButton_bed->isChecked())
    {
        vpList = getInputVar(var, bedfile);
    }

    return vpList;
}

QList<VariationPosition *> SNPSelectionDialog::filterSnps(QList<VariationPosition *> snpList, bool nonCodingExcluded, bool depthFilter, bool qualityFilter)
{
    qDebug() << "Variants filtering start... Number of Variants before filtering= " << snpList.size();
    QList<VariationPosition*> filteredSnps;
    int coding=0, depth=0, quality=0, indels=0;
    //nVars = snpList.size();
    int nfiltered;
    QList<VariationPosition*>::const_iterator i;
     for (i = snpList.begin(); i != snpList.end(); ++i)
      {
             VariationPosition *v = *i;

             VariationData gt = v->data[0];

             if (nonCodingExcluded && gt.coding == 0)// if nonCoding is excluded and snp type is noncoding
             {
                 //qDebug() << v->position << " is filtered by noncoding";
                 coding++;
                 continue;
             }

             if(depthFilter) //if depthFilter is on
             {
                 //qDebug() << v->position << " is filtered by depth";
                 long depth = ui->label_depth->text().toLong();
                 if(gt.readDepth<depth)
                 {
                     depth++;
                     continue;
                 }
             }

             if(qualityFilter) // if quality filter is on
             {
                 //qDebug() << v->position << " is filtered by quality";
                 double qual = ui->label_quality->text().toDouble();
                 if(v->quality<qual)
                 {
                     quality++;
                     continue;
                 }
             }

             filteredSnps.append(v);
        }
        nfiltered = filteredSnps.size();
        //nVars = nfiltered; // to let mainwindow (and/or) other(s) know the number of SNPs
        qDebug() << "Number of variants discarded from computation (if any) " ;
        if(ui->checkBox_noncoding->isChecked())qDebug() << "NonCoding:\t " << coding ;
        if(ui->checkBox_depth->isChecked()) qDebug() << "Below ReadDepth Threshold: " << depth ;
        if(ui->checkBox_qual->isChecked()) qDebug() << "Below Quality Threshold:" << quality;
        qDebug() << "Number of Variants after filtering= " << nfiltered;
        return filteredSnps;
}

QString SNPSelectionDialog::toInfoText(QString currentChrom, QStringList chromosomes, long from, long to, QString userText, QFile *bedfile)
{
    QString infoText="";

    if(ui->radiob_cview->isChecked())// snps in current view
    {
         infoText.append(QString("%1 : %2 to %3").arg(currentChrom).arg(from).arg(to));
    }
    else if(ui->radiob_chrom->isChecked())
    {
        foreach(QString chr, chromosomes)
                    infoText.append(chr+" ");
    }
    else  if(ui->radiob_custom->isChecked())
    {
        infoText = ui->plainTextEdit_selectedsnps->toPlainText().replace("\n"," ");
    }else if(ui->radioButton_bed->isChecked())
    {
        infoText = "BED File : " + ui->lineEdit_bedfile->text();
    }

    return infoText;
}

void SNPSelectionDialog::on_radiob_custom_clicked(bool checked)
{

}
