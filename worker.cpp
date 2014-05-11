#include "worker.h"
#include <QDebug>
#include <QMessageBox>

void Worker::doWork(){
    thetree = new TreeCalculation;
    successfullwork=false;
    compstarted=false;
    connect(thetree, SIGNAL(anotherSampleDone()), this, SIGNAL(incremented()));
    connect(thetree, SIGNAL(successAchieved()), this, SLOT(finishing()));
    connect(thetree, SIGNAL(processError(QString)), this, SLOT(emitErrorSignal(QString)));

    //NEVER USE QDEBUG OTHER THAN IN MAIN THREAD
    //qDebug() << " Worker::doWork()";
    emit started(thetree);
}

void Worker::startComputing(TreeCalculation *thetree)
{
    if(!compstarted)
    {
        emit treecomputingCalled();
        thetree->treecomputation();
        compstarted = true;
    }
}

void Worker::canceltree()
{
    thetree->stop();
}

Worker::~Worker()
{
    delete thetree;
}

void Worker::finishing()
{
    deleteLater();
    successfullwork=true;
    emit finishSignal();
}

void Worker::emitErrorSignal(QString err)
{
    emit errorOccur(err);
}
