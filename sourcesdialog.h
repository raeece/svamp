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
#ifndef SOURCESDIALOG_H
#define SOURCESDIALOG_H

#include <QDialog>

namespace Ui {
    class SourcesDialog;
}

class SourcesDialog : public QDialog {
    Q_OBJECT

public:
    explicit SourcesDialog(QWidget *parent = 0);
    ~SourcesDialog();
    void initialise(QString seq, QString anno, QString var, QString filter, QString db);
    void update(QString psFileType, QMap<QString,int>& paCounts);
    void update(QString psFileType, QString psType, int piCount);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SourcesDialog *ui;

private slots:
    int checkInputFiles();
    void writeDBPath();
    void on_varButton_clicked();
    void on_annoButton_clicked();
    void on_infoButton_clicked();
    void on_referenceButton_clicked();
    void on_okButton_clicked();
    void on_cancelButton_clicked();
    void on_dbButton_clicked();
};

#endif // SOURCESDIALOG_H
