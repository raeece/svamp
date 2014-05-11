#include "mapdialog.h"
#include "ui_mapdialog.h"
#include <QDebug>
#include <QMessageBox>

MapDialog::MapDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapDialog)
{
    ui->setupUi(this);

    mdh = new MapDataHandler();
     ui->webView->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled,true);
    if(mdh->dbConnected)
    {

        ui->webView->setPage(new MapPage(this));
        mdh->runQuery();
        QString content = mdh->composeCode();
        QWebFrame *frame = ui->webView->page()->mainFrame();
        frame->setHtml(content);

        QStringList year=mdh->queryYearRange();
        minYear=year.at(0).toInt();
        if(minYear<0)minYear=0;
        maxYear=year.at(1).toInt();

        int y=minYear;
        /*do{
            ui->comboBox_startYear->addItem(QString::number(y));
            y++;
        }while(y<=maxYear);*/

        //ui->comboBox_startYear->setCurrentIndex(0);
        ui->label_end->setText(QString::number(maxYear));
        ui->label_start->setText(QString::number(minYear));

        ui->horizontalSlider_start->setMinimum(minYear);
        ui->horizontalSlider_end->setMaximum(maxYear);

        ui->horizontalSlider_start->setValue(minYear);
        ui->horizontalSlider_end->setValue(maxYear);

        ui->horizontalSlider_start->setMaximum(ui->horizontalSlider_end->value());
        ui->horizontalSlider_end->setMinimum(ui->horizontalSlider_start->value());
        //ui->label_endYear->setText(QString::number(maxYear));

        /*ui->horizontalSlider->setMinimum(minYear);
        ui->horizontalSlider->setMaximum(maxYear);
        ui->horizontalSlider->setValue(maxYear);*/
        //ui->updatePushButton->setText("Within "+ QString::number(ui->horizontalSlider->value()));

        update();
        showMaximized();

    }
    else
    {
        QMessageBox nodb(QMessageBox::Critical, "No db Error ", "Please load sample information .db file", QMessageBox::Ok);
        nodb.exec();
        close();
    }
}

MapDialog::~MapDialog()
{
    delete ui;
    delete mdh;
}

/*void MapDialog::on_updatePushButton_clicked()
{
    mdh->updateQuery(" Year = "+ ui->label_endYear->text());
    QString content = mdh->composeCode();
    QWebFrame *frame = ui->webView->page()->mainFrame();
    frame->setHtml(content);
}*/

void MapDialog::on_pushButton_export_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Select where to save image file..."), "", "");

    if (0 == filename.length()) return;
    if (!(filename.endsWith(".png") || filename.endsWith(".jpg") || filename.endsWith(".bmp") || filename.endsWith(".jpeg") || filename.endsWith(".ppm") || filename.endsWith(".tiff") || filename.endsWith(".xbm") || filename.endsWith(".xpm")))
    {
        filename.append(".png");
    }

    QPixmap mapPM = QPixmap::grabWidget(ui->webView);
    QString label = "Range : " + ui->label_start->text() + " to " + ui->label_end->text();

    int fontSize = 12;
    QPainter p;
    QFont font;
    font.setPixelSize(fontSize);
    font.setFamily("Arial");

    int mapWidth = mapPM.width();
    int mapHeight = mapPM.height();

    QPixmap res( mapWidth, mapHeight+fontSize+4);
    res.fill(Qt::white);

    p.begin( &res );

    p.setFont(font);

    p.drawPixmap(0,0, mapPM );

    p.drawText(QRect(0,mapHeight+1,mapWidth, fontSize+2),0,label);

    p.end();

    res.save(filename);
}

void MapDialog::on_horizontalSlider_start_valueChanged(int value)
{
    ui->label_start->setText(QString::number(value));
    ui->horizontalSlider_end->setMinimum(value);
}

void MapDialog::on_horizontalSlider_end_valueChanged(int value)
{
    ui->label_end->setText(QString::number(value));
    ui->horizontalSlider_start->setMaximum(value);
}

void MapDialog::on_horizontalSlider_start_sliderReleased()
{
    mdh->updateQuery(" Year <= "+ ui->label_end->text() + " and Year >=" + ui->label_start->text());
    QString content = mdh->composeCode();
    QWebFrame *frame = ui->webView->page()->mainFrame();
    frame->setHtml(content);
}

void MapDialog::on_horizontalSlider_end_sliderReleased()
{
    mdh->updateQuery(" Year <= "+ ui->label_end->text() + " and Year >=" + ui->label_start->text());
    QString content = mdh->composeCode();
    QWebFrame *frame = ui->webView->page()->mainFrame();
    frame->setHtml(content);
}
