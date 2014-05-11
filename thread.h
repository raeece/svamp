#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <QProgressDialog>


class Thread: public QThread
{
     Q_OBJECT
public:
    Thread();

    void setMessage(const QString &message);

    void setParamaters(QString inFileName, QString outFileName, QString outTreeFile, QString spNum,  bool globalArr,
                       bool ranSp, QString inseedText, QString njumbleText, int methodCurrentIndex, bool negallow,
                       int dmatrixCurrentIndex, bool outgroupChecked, int noutgroup, int npower, bool replicatesChecked,
                       bool printtreeChecked, bool outtreeChecked);
    void treecomputation();
    void stop();
    void init();
    void maketree();

    int likemain(FILE * infilename, FILE * outfilename, FILE * outtreename);

    bool cancel, successfull;
    QProgressDialog progressdialog(QWidget * parent = 0, Qt::WindowFlags f = 0);

signals :
    void treebuildingstarted(long * total=0);
    void onesamplefinished();

protected:
    void run();

private:
    QString messageStr;
    volatile bool stopped;
    volatile bool started;
    QString inFileName, outFileName, outTreeFile, spNum;



};

#endif // THREAD_H
