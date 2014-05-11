#include "lookseqwindow.h"
#include "ui_lookseqwindow.h"
#include "mainwindow.h"
#include "QtWebKit"
#include <QDebug>
#include "mappage.h"

LookSeqWindow::LookSeqWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LookSeqWindow)
{
    ui->setupUi(this);
    ui->webView_lookseq->setPage(new MapPage(this));
    //ui->webView->setStyleSheet("background-color:rgb(150,147,88); padding: 7px ; color:rgb(255,255,255)");
     ui->webView_lookseq->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled,true);
     ui->webView_lookseq->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
     QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptEnabled, true);

}

void LookSeqWindow::locatePosition(int start, QString chromName, QStringList isolateID)
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    QString filePath="";
    filePath.append(PREFIX);
    filePath.append("svamp.temp");

    QFile temp(filePath);
    QString path;
    if(temp.open(QIODevice::ReadOnly| QIODevice::Text))
    {
        QTextStream fin(&temp);
        fin.readLine();//the first line is db path
        path = fin.readLine();
        temp.close();
    }

    path.append("&loc=");
    path.append(chromName);
    path.append(":");

    int end=start+72;
    QString startPosition= QString::number(start);
    path.append(startPosition);
    path.append("..");
    QString endPosition= QString::number(end);

    path.append(endPosition);
    path.append("&tracks=DNA,");
    QString label;
    int nSelectedSample=isolateID.size();
    for(int i=0;i<nSelectedSample;i++)
    {
        QString id=isolateID.at(i);
        label= id+" SNPCoverage,"+id+" Alignment";

        if(i<nSelectedSample-1)label+=",";

        path.append(label);
    }
    path.append("&highlight=");

//    path.append(isolateID);
//    path.append("|smalt&options=perfect|1%2Csnps|1%2Csingle|0%2Cinversions|1%2Cpairlinks|1%2Corientation|0%2Cfaceaway|0%2Cbasequal|0&maxdist=300");
//    //ui->webView_lookseq->load(path);
    qDebug() << "loaded path : " << path;
    QUrl builtUrl = QUrl(path);
    qDebug() << "Built Url" << builtUrl.toString();
    QDesktopServices::openUrl(path);
    //QDesktopServices::openUrl(QUrl("http://localhost/JBrowse1.9/index.html?data=tw20_full&loc=TW20_full:1225581..1232830&tracks=DNA,AGT1 Alignment,AGT1 SNPCoverage&highlight=", QUrl::TolerantMode));
}

LookSeqWindow::~LookSeqWindow()
{
    delete ui;
}
