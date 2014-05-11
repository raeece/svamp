#ifndef TREEOPTIONSDIALOG_H
#define TREEOPTIONSDIALOG_H

#include <QDialog>
#include "treecalculation.h"
#include "worker.h"
#include "mainwindow.h"
#include "variation.h"

namespace Ui {
class TreeOptionsDialog;
}

class TreeOptionsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit TreeOptionsDialog(QWidget *parent = 0);
    ~TreeOptionsDialog();
    bool showMore;

    void closeEvent(QCloseEvent *);

signals:
    void optionsSaved(TreeCalculation * tree);
    void cancelled();
    void activateTreeMenu();
    
private slots:

    void on_comboBox_method_currentIndexChanged(int index);

    void GetOptions(TreeCalculation * threadtree);

    void on_pushButton_more_clicked();

    void on_pushButton_ok_clicked();

    void incrementDone();

    void viewTree();

    void fireErrorMessage(QString err);

    void computeTree(QList<VariationPosition *> vpList, bool nonCoding,bool selectedsamples);

    void saveAlignment(QList<VariationPosition *> vpList, bool nonCoding,bool selectedsamples);

    int generatePhylipMatrix(QList<VariationPosition *> vpList, bool nonCoding,QStringList samplelist);

    void on_comboBox_sampleList_currentIndexChanged(const QString &arg1);

    void on_pushButton_display_tree_clicked();

    void on_pushButton_save_clicked();

    void on_pushButton_choosedirectory_clicked();

    void on_pushButton_choosefile_clicked();

private:
    Ui::TreeOptionsDialog *ui;
    int donesamples;
    QProgressDialog * progressdialog;
    Worker *worker ;

    QList<QString> leftalign(QString r, QString s);
    QList<int> getEdit(QString ref);
    QString applyedit(QString ref,QList<int> edits);
    QList<int> translateEdit(QList<int> originaledit,QList<int> appliededit);
    QList<QString> leftMSA(QString ref,QList <QString> samples);
};

#endif // TREEOPTIONSDIALOG_H
