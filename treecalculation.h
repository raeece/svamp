#ifndef THREAD_H
#define THREAD_H

#include <QProgressDialog>
#include <QString>
#include <QObject>

class TreeCalculation : public QObject
{
    Q_OBJECT
public:
    void setMessage(const QString &message);

    void setParamaters(QString inFileName, QString outFileName, QString outTreeFile, long spNum,  bool globalArr,
                       bool ranSp, int methodCurrentIndex, bool negallow, int dmatrixCurrentIndex, int npower,
                       bool replicatesChecked, bool printtreeChecked, bool outtreeChecked);

    bool cancel, successfull;

public slots:
    void treecomputation();
    void stop();
    void init();
    int maketree();
    int likemain(FILE * infilename, FILE * outfilename, FILE * outtreename);

signals:
    void anotherSampleDone();
    void successAchieved();
    void processError(QString err);

private:
    QString messageStr;
    QString inFileName, outFileName, outTreeFile;
    long spNum;

};

#endif // THREAD_H
