#include "track.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "save.h"
#include "ui_save.h"
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>

Save::Save(QWidget *parent) : QDialog(parent), ui(new Ui::Save) {
    ui->setupUi(this);
    QFile file("save.pv");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream fin(&file);
        ui->dataFileLine->setText(fin.readLine());
        ui->imagesFileLine->setText(fin.readLine());
        file.close();
    }
}
void Save::saveFilenames() {
    QFile file("save.pv");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream fout(&file);
        fout << ui->dataFileLine->text() << endl;
        fout << ui->imagesFileLine->text() << endl;
        file.close();
    }
}

Save::~Save() {
    delete ui;
}

void Save::on_saveDataButton_clicked() {
    QString filename = QFileDialog::getSaveFileName(this, tr("Select data file to save to"), ui->dataFileLine->text(), "*.csv");
    if (0 == filename.length()) return;
    close();
    repaint();
    ui->dataFileLine->setText(filename);
    saveFilenames();
    Feedback* feedback = &(MainWindow::instance->feedback);
    feedback->initialise(1);
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream fout(&file);
        feedback->update(1,"Header");
        TrackContainer *tracks = MainWindow::instance->getUI()->tracks;
        TrackVariation *trackvar = (TrackVariation*) tracks->getTrack("variation");
        QString chromosome = trackvar->range.chromosome;
        long from = trackvar->range.from, to = trackvar->range.to;
        fout << "Parameters:" << endl;
        fout << "chromosome    " << chromosome    << endl;
        fout << "from          " << from          << endl;
        fout << "to            " << to            << endl;
        fout << "quality       " << trackvar->range.quality       << endl;
        fout << "readDepth     " << trackvar->range.readDepth     << endl;
        fout << "nonSynonymous " << trackvar->range.nonSynonymous << endl;
        fout << "synonymous    " << trackvar->range.synonymous    << endl;
        fout << "insertions    " << trackvar->range.insertion     << endl;
        fout << "deletions     " << trackvar->range.deletion      << endl;
        fout << endl;

        feedback->update(1,"Groups");
        Variation *var = tracks->getVariation();
        foreach (QString gp, var->groups.keys()) {
            fout << "Groups:" << endl;
            for (long gpi = 0; gpi < var->groups[gp].size(); gpi++) {
                fout << gp << " " << var->samples[var->groups[gp][gpi]] << endl;
            }
        }
        fout << endl;

        for (long i = 0; i < tracks->tracklist.size(); i++) {
            TrackGraph *statistic = NULL;
            statistic = dynamic_cast<TrackGraph*>(tracks->tracklist[i]);
            if (statistic) {
                fout << "Statistic: " << statistic->title << endl;
                feedback->update(1,statistic->title);
                feedback->update(2,"-");
                GraphData *d = statistic->data[chromosome];
                int liCount = 0;
                for (long j = 0; j < statistic->size[chromosome]; j++) {
                    if (d[j].position < from) continue;
                    if (d[j].position > to)   break;
                    if (d[j].data     == 0)   continue;
                    fout << d[j].position << " ";
                    if (0 == (liCount % 1000)) feedback->update(2,QString::number(liCount));
                    liCount++;
                }
                fout << endl;
                for (long j = 0; j < statistic->size[chromosome]; j++) {
                    if (d[j].position < from) continue;
                    if (d[j].position > to)   break;
                    if (d[j].data     == 0)   continue;
                    fout << d[j].data << " ";
                    if (0 == (liCount % 1000)) feedback->update(2,QString::number(liCount));
                    liCount++;
                }
                feedback->update(2,QString::number(liCount));
                fout << endl << endl;
                fout.flush();
            }
        }
        file.close();
        feedback->update(1,"Closing");
        feedback->update(2, "-");
        feedback->clear();
    } else {
        QMessageBox msgBox;
        msgBox.setText("File not opened - " + filename);
        msgBox.exec();
    }
}

void Save::on_saveImagesButton_clicked() {
    QString filename;
    if(ui->imagesFileLine->text().isEmpty())
        filename = QFileDialog::getSaveFileName(this, tr("Select folder and write filename"), ui->imagesFileLine->text(), "*.png");
    else
        filename = ui->imagesFileLine->text();

    if (0 == filename.length()) return;
    if (!(filename.endsWith(".png") || filename.endsWith(".jpg") || filename.endsWith(".bmp") || filename.endsWith(".jpeg") || filename.endsWith(".ppm") || filename.endsWith(".tiff") || filename.endsWith(".xbm") || filename.endsWith(".xpm")))
    {
        filename.append(".png");
    }

    ui->imagesFileLine->setText(filename);
    saveFilenames();

    MainWindow::instance->exportView(filename);
    close();
}

void Save::on_cancelButton_clicked() {
    close();
}
