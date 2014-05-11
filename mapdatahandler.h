#ifndef MAPDATAHANDLER_H
#define MAPDATAHANDLER_H

#include <QtSql>
#include <QFileInfo>
#include "dbquery.h"

class MapDataHandler : public QObject
{
    Q_OBJECT

public:
    MapDataHandler();
    ~MapDataHandler();

    QString composeCode();
    void runQuery();
    void updateQuery(QString condition);
    QStringList queryYearRange();
    bool dbConnected;

signals:
    void statusUpdate(QString status);

private:
    QString queryContent, centerlat,centerlng;
    QString addInfoWindow(QString place);
    QString addFullInfoWindow(QString place);
    QString addFullInfoWindow(QString place, QString condition);
    DBQuery dbq;
};

#endif // MAPDATAHANDLER_H
