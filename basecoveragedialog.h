#ifndef BASECOVERAGEDIALOG_H
#define BASECOVERAGEDIALOG_H

#include <QDialog>
#include "trackcontainer.h"
#include "sequence.h"
#include "variation.h"

namespace Ui {
class BaseCoverageDialog;
}

class BaseCoverageDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit BaseCoverageDialog(QWidget *parent = 0);
    ~BaseCoverageDialog();
    
private slots:
    void on_browseButton1_clicked();
    void on_okButton_clicked();
    //void on_addButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::BaseCoverageDialog *ui;
    TrackContainer *area;
    Sequence * seq; //to retrieve name of chromosome to be listed in combobox
    Variation * var; //to run loadCover function which is in class Variation
};

#endif // BASECOVERAGEDIALOG_H
