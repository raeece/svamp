#ifndef PCOADIALOG_H
#define PCOADIALOG_H

#include <QDialog>
#include <QtCore>
#include <QtGui>
#include "ap.h"
#include "point.h"
#include "variation.h"
#include "trackcontainer.h"
#include "dbquery.h"
#include "gridscene.h"

namespace Ui {
class PCOADialog;
}

class PCOADialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit PCOADialog(QWidget *parent = 0);
    alglib::real_2d_array finDa;
    QList<QString> selectedSampleNamesList;
    Ui::PCOADialog* getUI ();

    void initData(alglib::real_2d_array finDa, bool selectedOnly, QList<QString> selectedSampleNamesList);

    void showLegend();

    QTextBrowser * getTextBrowser1();
    QTextBrowser * getTextBrowser2();

    QPointF LastPoint;

    ~PCOADialog();

signals:
    void scrolled();
    
private:
    Ui::PCOADialog *ui;

    TrackContainer *areas;
    Variation * vars;
    bool selectedOnly, panning;

    double min_x, max_x, min_y, max_y;
    qreal xZero,yZero,xScale,yScale;

    int currentPortWidth, currentPortHeight;

    QGraphicsScene *scene_x, *scene_y;
    GridScene * scene;
    QGraphicsLineItem *xAxis;
    QGraphicsLineItem *yAxis;
    QGraphicsLineItem *horizontalBorder;
    QGraphicsLineItem *verticalBorder;

    QList<Point * > toBeVisible;

//protected:
//    void closeEvent(QCloseEvent *ev);

private slots:
    void plotAxis(int portwidth, int portheight/*, alglib::real_2d_array fd*/);
    void plotPoints(alglib::real_2d_array finDa, qreal xZero, qreal yZero, qreal xScale, qreal yScale, bool selectedOnly, QList<QString> selectedSampleNamesList);
    void zoomIn();
    void zoomOut();
    void adjustScrollBar();
    void zoomPlot();
    void on_pushButton_save_clicked();
};

#endif // PCOADialog_H
