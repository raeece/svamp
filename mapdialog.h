#ifndef MAPDIALOG_H
#define MAPDIALOG_H

#include <QDialog>
#include <QWebFrame>
#include "mapdatahandler.h"
#include "mappage.h"

namespace Ui {
class MapDialog;
}

class MapDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit MapDialog(QWidget *parent = 0);
    ~MapDialog();
    
private slots:

    //void on_updatePushButton_clicked();

    void on_pushButton_export_clicked();

    void on_horizontalSlider_start_valueChanged(int value);

    void on_horizontalSlider_end_valueChanged(int value);

    void on_horizontalSlider_start_sliderReleased();

    void on_horizontalSlider_end_sliderReleased();

private:
    Ui::MapDialog *ui;
    MapDataHandler * mdh;

    int maxYear, minYear;
};

#endif // MAPDIALOG_H
