#include "dbquery.h"

DBQuery::DBQuery()
{
    if (QSqlDatabase::contains("qt_sql_default_connection"))
        myDB = QSqlDatabase::database("qt_sql_default_connection");
    else
        myDB = QSqlDatabase::addDatabase("QSQLITE"); //connect to driver
}

void DBQuery::connect()
{    
    //QDir::setCurrent(QCoreApplication::applicationDirPath());

    QFile temp("svamp.temp");
    if(temp.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream fin(&temp);
        dbPath = fin.readLine();

        if(dbPath.isEmpty())
            this->closeDB();
        else
            myDB.setDatabaseName(dbPath);
        temp.close();
    }
    QFileInfo checkFile(dbPath);
    if(checkFile.isFile())
    {
        if(myDB.open())
        {
            emit statusUpdate("Database connection successfull");
            qDebug() << "Database connection succesfull";
        }
    }else{
        emit statusUpdate("Database file doesn't exist");
        qDebug() << "Db file not found";
    }

}

void DBQuery::closeDB()
{
    myDB.close();
}

void DBQuery::updateGroupingInDB(QStringList sampleList, QString group, QString subgroup)
{
    QString locationId;

    if(!myDB.isOpen()){
        qDebug() << "DBQuery::updateGroupingInDB No Connection to DB";
        emit statusUpdate("No connection to db :( " );
    }else
    {
            QSqlQuery qry;

            if(qry.exec("select id from location where country=\'" + group + "\' and place=\'" +subgroup+ "\' ;"))
            {
                if(qry.next())
                {
                    locationId = qry.value(0).toString();
                    //check if samples exists and then update it in the sample table
                    foreach(QString sample, sampleList)
                    {
                        qry.exec("select 1 from samples where name=\'"+ sample + "\' limit 1");
                        if(qry.next())
                        {
                            qry.exec("update samples set origin_id=" +locationId+ " where name= \'"+ sample + "\' ;");
                        }else
                        {
                            qry.exec("select count(*) from samples;");
                            if(qry.next())
                            {
                                int nRow = qry.value(0).toInt();
                                int num=1, year=0;
                                if(nRow>0)
                                {
                                    qry.exec("select max(ID) from samples;");
                                    if(qry.next())
                                    {
                                        num= qry.value(0).toInt()+1;
                                    }else
                                        qDebug() << "Max ID for table Sample not updated";
                                }
                                qry.exec("insert into samples values("+ QString::number(num) + ", \'" +sample+ "\'," + QString::number(year) + ", " +locationId+ ");" );
                            }
                        }
                    }
                }else
                {
                    qDebug() << "no result for query locationId in updateGroupingInDB";
                }
            }else
            {
                qDebug() << "query failed in DBQuery::updateGroupingInDB" << qry.lastError();
            }
    }/*end of else*/
}

QStringList DBQuery::querySubGroupList(QString group)
{
    QStringList placeList;
    if(!myDB.isOpen()){
        emit statusUpdate("No connection to db :( " );
    }else
    {
        QSqlQuery sql;
        if(sql.exec("select place from location where country= \'" + group + "\' ;"))
        {
            while(sql.next())
            {
                QString place = sql.value(0).toString();
                placeList.append(place);
            }//end of while
        }//end of if exec

    }
    return placeList;
}

/*QMap<QString,double> DBQuery::queryAlleleFreq(int position)
{
    QMap<QString, double> allelfreq;
    QStringList alleleList = this->queryAlleles(position);
    foreach(QString allele, alleleList)
        allelfreq.insert(allele,0);

    if(!myDB.isOpen())
    {
         emit statusUpdate("No connection to db :( " );
    }else
    {
        QSqlQuery sql;
        double total=0;
        QMap<QString,double> rs;
        if(sql.exec("select base, count(base) as tot from variationtransaction where position=" + QString::number(position) + " group by base;"))
        {
            while(sql.next())
            {
                rs.insert(sql.value(0).toString(),sql.value(1).toDouble());
                total+=sql.value(1).toDouble();
            }
            foreach(QString key, rs.keys())
                allelfreq[key]=rs[key]/total;
        }
    }
    return allelfreq;
}*/

int DBQuery::insertIntoLocation(QString city, QString group, double latitude, double longitude)
{
    if(!myDB.isOpen()){
        emit statusUpdate("No connection to db :( " );
        return -1;
    }else
    {
        int maxid;
        QSqlQuery qry;
        qry.exec("select max(id) from location;");
        if(qry.next())
        {
            maxid=qry.value(0).toInt();
            if(maxid<0) maxid =0 ;
        }
        else
            maxid=0;

        qry.prepare("insert into location values(:max, :city, :group , :latitude , :longitude);");
        qry.bindValue(":max", maxid+1);
        qry.bindValue(":city", city);
        qry.bindValue(":group" ,group);
        qry.bindValue(":latitude",QString::number(latitude));
        qry.bindValue(":longitude", QString::number(longitude));

        if(!qry.exec())
        {
            qDebug() << "Unable to create group: " << qry.lastError() << " last statement : " <<qry.lastQuery();
            return -1;
        }else
        {
            qDebug() << "New group created : " << group;
            return 0;
        }

    }/*end of else*/
}

bool DBQuery::deleteGroup(QString groupName)
{
    if(!myDB.isOpen()){
        emit statusUpdate("No connection to db :( " );
        return false;
    }else
    {
        QSqlQuery qry;
        qry.prepare("delete from location where country= :name ;");
        qry.bindValue(":name" ,groupName);

        if(!qry.exec())
        {
            qDebug() << "deletion of group in DB is unsuccessfull : " <<qry.lastQuery();
            return false;
        }else
            return true;
    }/*end of else*/
}

bool DBQuery::isConnected()
{
    return myDB.isOpen();
}

bool DBQuery::createNewDB(QString DBpath)
{
    if (myDB.isOpen())
    {
        this->closeDB();
        myDB = QSqlDatabase::addDatabase("QSQLITE"); //connect to driver
    }

    myDB.setDatabaseName(DBpath);
    return myDB.open();
}

bool DBQuery::isUnique(QString group, QString subgroup)
{
    if(this->isConnected())
    {
        QSqlQuery qry;
        if(qry.exec("select ID from location where place=\'" + subgroup + "\' and country=\'" + group + "\';"))
        {
            if(qry.next())
            {
               return false;
            }else
                return true;
        }else
            qDebug() << "isUnique query failed to execute";

    }else
        qDebug() << "DBQuery::isUnique << Database is not connected ";
    return false;
}

void DBQuery::createTables()
{
    QSqlQuery qry;
    //qry.exec("CREATE TABLE variationTransaction(position int, sample varchar(255), haplotype int, base varchar(2), chrom varchar(255));");
    qry.exec("CREATE TABLE location(ID int,place varchar(255),country varchar(255), latitude double, longitude double);");
    qry.exec("CREATE TABLE Samples (ID int, Name varchar(255), Year int, Origin_id int);");

    //insert "ungrouped" as one record of table location (by default)
    qry.exec("insert into location values(1,\'Ungrouped\', \'Ungrouped\', -, -);");
}

QStringList DBQuery::queryAlleles(int position)
{
    QStringList alt;
    if(this->isConnected())
    {
        QSqlQuery qry;
        if(qry.exec("select distinct base from variationTransaction where position=" +QString::number(position) + ";"))
        {
            if(qry.next())
            {
                do
                {
                    alt.append(qry.value(0).toString());
                }while(qry.next());
            }
        }
    }
    return alt;
}

QStringList DBQuery::querySampleList(QString group, QString subgroup)
{
    //qDebug() << "DBQuery::querySampleList";
    QStringList sampleList;
    if(this->isConnected())
    {
        QSqlQuery qry;
        if(qry.exec("select samples.name from samples,location where samples.origin_id = location.id and location.place=\'" +subgroup+ "\' and location.country=\'"+ group +"\'; "))
        {
            while(qry.next())
            {
                    sampleList.append(qry.value(0).toString());
            }
        }
    }
    return sampleList;
}

QMap<QString, QStringList> DBQuery::querySampleOrigin()
{
    QMap<QString, QStringList> sampleOriginPair;

    if(!myDB.isOpen()){
        emit statusUpdate("No connection to db :( " );
    }else
    {
        QSqlQuery sampleOriginQry;
        if(sampleOriginQry.exec("select country, name from location left join samples on origin_id=location.id;"))
        {
            QStringList sampleList;
            while(sampleOriginQry.next())
            {
                QString country = sampleOriginQry.value(0).toString();
                QString sampleName = sampleOriginQry.value(1).toString();

                if(sampleOriginPair.contains(country))
                {
                    sampleOriginPair[country].append(sampleName);
                }else
                {
                    sampleList.append(sampleName);
                    sampleOriginPair.insert(country, sampleList);
                    sampleList.clear();
                }
            }
        }else
        {
            qDebug() << sampleOriginQry.lastError();
        }

    }/*end of else*/
    return sampleOriginPair;
}

