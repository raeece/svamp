#include "mapalleledialog.h"
#include "ui_mapalleledialog.h"
#include "mainwindow.h"
#include <QDebug>
#include <QtNetwork/QNetworkReply>
#include "time.h"

MapAlleleDialog::MapAlleleDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapAlleleDialog)
{
    ui->setupUi(this);
    ui->vSlider_bChart->setValue(90);

    databaseOK=false;
    dpm = new DojoPieMap(this);
    if(dpm->dbq.isConnected()){
        databaseOK=true;
    }
    else{
        return;
    }

    connect (dpm, SIGNAL(databaseStatus(QString)), this, SLOT(updateDataStatus(QString)));

    QProgressDialog progressdialog(MainWindow::instance);
    progressdialog.setModal(false);
    progressdialog.setRange(0,4);

    ui->webView->setPage(new MapPage(this));
    QWebFrame *frame = ui->webView->page()->mainFrame();
    ui->webView->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled,true);
    ui->webView_barchart->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

    progressdialog.setLabelText(tr("Generating Allele Map..."));
    progressdialog.setValue(1);
    qApp->processEvents();
    progressdialog.show();

    QString html = dpm->generateHTML();
    if (html.isEmpty())
    {
        QMessageBox warning(QMessageBox::Critical, "Map Error ", "Query returns empty result", QMessageBox::Ok);
        warning.exec();
        close();
        return;
    }

    frame->setHtml(html,QUrl("http://openlayers.org/api/2.10/OpenLayers.js"));

    progressdialog.setValue(2);
    qApp->processEvents();
    progressdialog.setLabelText("Calculating Map Statistics...");
    ui->webView_barchart->setHtml(dpm->drawBarChart(), QUrl("http://ajax.googleapis.com/ajax/libs/dojo/1.7.1/dojo/dojo.js") );

    progressdialog.setValue(3);
    qApp->processEvents();
    progressdialog.setLabelText("Preparing Window...");
    qApp->processEvents();

    int pos = dpm->getPosition();
    QStringList alleList = dpm->dbq.queryAlleles(pos);
    ui->label_snppos->setText("Variant Position : " + QString::number(pos));

    int i=0;
    foreach(QString a, alleList)
    {
        QListWidgetItem * item = new QListWidgetItem;
        item->setText(a);
        item->setFont(QFont("Comic Sans",14));
        item->setTextColor(getColor(i));
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(Qt::NoItemFlags);
        ui->listWidget_legend->addItem(item);
        i++;
    }
     progressdialog.setLabelText("Drawing...");
     qApp->processEvents();
     progressdialog.hide();
}

MapAlleleDialog::~MapAlleleDialog()
{
    delete ui;
}

QColor MapAlleleDialog::getColor(int index)
{
    QColor c;
    switch(index){
    case 0:
    {
        c.setNamedColor("blue");
        break;
    }
    case 1:
    {
        c.setNamedColor("red");
        break;
    }
    case 2:
    {
        c.setNamedColor("green");
        break;
    }
    case 3:
    {
        c.setNamedColor("purple");
        break;
    }
    }
    return c;
}

void MapAlleleDialog::updateDataStatus(QString status)
{
    ui->label_status->setText(status);
}

void MapAlleleDialog::on_pushButton_exportMap_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Select where to save image file..."), "", "");

    if (0 == filename.length()) return;
    if (!(filename.endsWith(".png") || filename.endsWith(".jpg") || filename.endsWith(".bmp") || filename.endsWith(".jpeg") || filename.endsWith(".ppm") || filename.endsWith(".tiff") || filename.endsWith(".xbm") || filename.endsWith(".xpm")))
    {
        filename.append(".png");
    }

    QPixmap mapPM = QPixmap::grabWidget(ui->webView);
    QString label = ui->label_snppos->text();

    int fontSize = 12;
    QPainter p;
    QFont font;
    font.setPixelSize(fontSize);
    font.setFamily("Arial");

    int mapWidth = mapPM.width();
    int mapHeight = mapPM.height();

    QPixmap res( mapWidth, mapHeight+fontSize+2);
    res.fill(Qt::white);

    p.begin( &res );

    p.setFont(font);

    p.drawPixmap(0,0, mapPM );

    p.drawText(QRect(0,mapHeight+1,mapWidth, fontSize),0,label);

    p.end();

    res.save(filename);
}

void MapAlleleDialog::on_pushButton_exportChart_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Select where to save image file..."), "", "");

    if (0 == filename.length()) return;
    if (!(filename.endsWith(".png") || filename.endsWith(".jpg") || filename.endsWith(".bmp") || filename.endsWith(".jpeg") || filename.endsWith(".ppm") || filename.endsWith(".tiff") || filename.endsWith(".xbm") || filename.endsWith(".xpm")))
    {
        filename.append(".png");
    }

    QPixmap mapPM = QPixmap::grabWidget(ui->webView_barchart);
    QString label = ui->label_snppos->text();

    int fontSize = 12;
    QPainter p;
    QFont font;
    font.setPixelSize(fontSize);
    font.setFamily("Arial");

    int mapWidth = mapPM.width();
    int mapHeight = mapPM.height();

    QPixmap res( mapWidth, mapHeight+fontSize+2);
    res.fill(Qt::white);
    p.begin( &res );
    p.setFont(font);
    p.drawPixmap(0,0, mapPM );
    p.drawText(QRect(0,mapHeight+1,mapWidth, fontSize),0,label);
    p.end();
    res.save(filename);
}

void MapAlleleDialog::on_vSlider_bChart_valueChanged(int value)
{
#if QT_VERSION >= 0x040500
    ui->webView_barchart->setZoomFactor(value/90.0);
#else
    ui->webView_barchart->setTextSizeMultiplier(value/90.0);
#endif
}
