#ifndef TREEVIEWDIALOG_H
#define TREEVIEWDIALOG_H

#include <QDialog>
#include "trackcontainer.h"
#include "variation.h"

namespace Ui {
    class TreeViewDialog;
}

class TreeViewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TreeViewDialog(QWidget *parent = 0);
    ~TreeViewDialog();

void showTree(QString newickString, QString chr, int from, int to);

private:
    Ui::TreeViewDialog *ui;
    TrackContainer *track;
    Variation *vars;
    QString newickString;
    QString newickToXML(QString ,QHash<QString,QString> sample_group,QHash<QString,QString>);
    QString generateHTML(int, int,int);

private slots:
   void on_checkBox_stateChanged(int arg1);
   void on_spinBox_valueChanged(int arg1);
   void on_pushButton_save_clicked();
};

#endif // TREEVIEWDIALOG_H
