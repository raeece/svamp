#ifndef COMBINEDIALOG_H
#define COMBINEDIALOG_H

#include <QDialog>
#include "trackcontainer.h"
#include "variation.h"
#include "sequence.h"
#include <QtGui/QLineEdit>

namespace Ui {
class CombineDialog;
}

class CombineDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CombineDialog(QWidget *parent = 0);
    ~CombineDialog();
    
private slots:
    void on_browseButton_1_clicked();

    void on_buttonBox_accepted();

    void on_addButton_clicked();

    void getFile();

private:
    Ui::CombineDialog *ui;
    TrackContainer *area;
    Sequence *seq;// to retrieve the chromosome name list
    QList<QLineEdit *> lineEditList; // to store pointer to lineEdit instead of creating a list of lineEdit.
    QList<QPushButton *> pushButtonList; // same as above
    int layoutCount;
};

#endif // COMBINEDIALOG_H
