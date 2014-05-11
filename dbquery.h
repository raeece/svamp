#ifndef DBQUERY_H
#define DBQUERY_H

#include <QtSql>
#include <QFileInfo>
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "variation.h"

class DBQuery : public QObject
{
    Q_OBJECT

public:
    DBQuery();
    QMap<QString, QStringList> querySampleOrigin();
    void writeDBPath(QString path);
    void connect();
    void closeDB();
    void updateGroupingInDB(QStringList sampleList, QString group, QString subgroup);
    QStringList querySubGroupList(QString group);
    int insertIntoLocation(QString city, QString group, double latitude, double longitude);
    bool deleteGroup(QString groupName);
    bool isConnected();
    bool createNewDB(QString DBpath);
    bool isUnique(QString group, QString subgroup);
    void createTables();
    QStringList queryAlleles(int position);
    QStringList querySampleList(QString group, QString subgroup);

signals:
    void statusUpdate(QString status);

private:
    QString dbPath;
    QSqlDatabase myDB;


};

#endif // DBQUERY_H
