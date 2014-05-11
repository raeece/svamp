#ifndef DOJOPIEMAP_H
#define DOJOPIEMAP_H

#include "dbquery.h"

typedef struct{
    QString name;
    double latitude;
    double longitude;
    QMap<QString, int> altCount;
} originMetaData;

class DojoPieMap : public QObject
{
    Q_OBJECT
public:
    explicit DojoPieMap(QObject *parent = 0);
    ~DojoPieMap();

    QString generateHTML();
    int getPosition();
    void setChromosome(QString chrom);
    QString drawBarChart();
    DBQuery dbq;

signals :
    void databaseStatus(QString status);

private:
    int position;

    void setPosition(long position);
    QString centerlat, centerlng, chrom;
    QString generateScript();
    QList<originMetaData>  mapData;

    QList<originMetaData> queryMetaData();


};

#endif // DOJOPIEMAP_H
