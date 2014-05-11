#ifndef MAPALLELEDIALOG_H
#define MAPALLELEDIALOG_H

#include <QDialog>
#include "mapdatahandler.h"
#include "mappage.h"
#include "dojopiemap.h"
#include <QWebFrame>

namespace Ui {
class MapAlleleDialog;
}

class MapAlleleDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit MapAlleleDialog(QWidget *parent = 0);
    ~MapAlleleDialog();
    bool databaseOK;
    
private:
    Ui::MapAlleleDialog *ui;
    DojoPieMap * dpm;
    QColor getColor(int index);

private slots:
    void updateDataStatus(QString status);
    void on_pushButton_exportMap_clicked();
    void on_pushButton_exportChart_clicked();
    void on_vSlider_bChart_valueChanged(int value);
};

#endif // MAPALLELEDIALOG_H
