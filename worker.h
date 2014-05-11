#ifndef WORKER_H
#define WORKER_H
#include <QObject>
#include "treecalculation.h"

class Worker : public QObject
{
    Q_OBJECT
public :
    void canceltree();
    ~Worker();
    bool successfullwork;
    void doWork();

signals:
    void incremented();
    void started(TreeCalculation * tree);
    void workcancelled();
    void finishSignal();
    void errorOccur(QString err);
    void treecomputingCalled();

private :
    TreeCalculation * thetree;
    bool compstarted;

private slots:
    //void doWork();
    void startComputing(TreeCalculation * thetree);
    void finishing();
    void emitErrorSignal(QString err);
};


#endif // WORKER_H
