#ifndef SNPSELECTIONDIALOG_H
#define SNPSELECTIONDIALOG_H

#include <QDialog>
#include "variation.h"
#include <QFile>

namespace Ui {
class SNPSelectionDialog;
}

class SNPSelectionDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SNPSelectionDialog(QWidget *parent = 0);
    ~SNPSelectionDialog();
    void changeButtonText();

    bool areAllSamples, selectedsnp, pcoa,savemode,selectedsamples;
    
signals:
    void startTreeComputation(QList<VariationPosition *> vpList, bool nonCoding,bool selectedsamples);
    void startSaveAlignment(QList<VariationPosition *> vpList, bool nonCoding,bool selectedsamples);

private slots:
    void on_pushButton_save_clicked();

    void on_radiob_chrom_toggled(bool checked);

    void on_radiob_custom_toggled(bool checked);

    void on_radiob_cview_toggled(bool checked);

    void on_lineEdit_bedfile_selectionChanged();

    void on_radioButton_bed_toggled(bool checked);

    QList<VariationPosition *> getInputVar(Variation * var, QStringList chromosome);
    QList<VariationPosition *> getInputVar(Variation * var, QString chromosome, long from, long to);
    QList<VariationPosition *> getInputVar(Variation * var, QString chromosome, QString userText);
    QList<VariationPosition *> getInputVar(Variation *var,  QFile * bedfile );

    QList<VariationPosition *> getVarPosBetween(Variation * var, QString chromosome,long x1, long x2);

    QList<VariationPosition *> prepareSnpList(Variation *var, QString currentChrom, QStringList chromosomes, long from, long to, QString userText, QFile *bedfile);

    QList<VariationPosition *> filterSnps(QList<VariationPosition *> snpList, bool nonCoding, bool depthFilter, bool qualityFilter);

    QString toInfoText(QString currentChrom, QStringList chromosomes, long from, long to, QString userText, QFile *bedfile);

    void on_radiob_custom_clicked(bool checked);


private:
    Ui::SNPSelectionDialog *ui;
};

#endif // SNPSELECTIONDIALOG_H
