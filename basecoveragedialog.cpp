#include "basecoveragedialog.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_basecoveragedialog.h"

BaseCoverageDialog::BaseCoverageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BaseCoverageDialog)
{
    ui->setupUi(this);
    area = ((MainWindow*)parent)->getUI()->tracks;
    seq = area->getSequence();
    var = area->getVariation();
    QList <QString> chrlist;
    chrlist = seq->chromosomes.keys();
    ui->chromComboBox->clear();
    ui->chromComboBox->addItems(chrlist);
}

BaseCoverageDialog::~BaseCoverageDialog()
{
    delete ui;
}

void BaseCoverageDialog::on_browseButton1_clicked()
{
    QString f = QFileDialog::getOpenFileName (this, tr("Select coverage file"), ui->coverageFile1Line->text(), tr("COV Files (*.cov)"));
    if (!f.isNull()) ui->coverageFile1Line->setText(f);
}

void BaseCoverageDialog::on_okButton_clicked()
{
    var->loadCover(ui->coverageFile1Line->text(), ui->chromComboBox->currentText());
    close();
}

void BaseCoverageDialog::on_pushButton_2_clicked()
{
    this->close();
}
