#include "dojopiemap.h"
#include <QSqlQuery>
#include <QFile>
#include "time.h"

DojoPieMap::DojoPieMap(QObject *parent) :
    QObject(parent)
{
    dbq.connect();

    position =MainWindow::instance->clickedPosition;
    setPosition(position);
    setChromosome(MainWindow::instance->getUI()->chrBox->currentText());
}

DojoPieMap::~DojoPieMap()
{
    dbq.closeDB();
}

void DojoPieMap::setPosition(long position)
{
    this->position = position;
}

void DojoPieMap::setChromosome(QString chrom)
{
    this->chrom = chrom;
}

QString DojoPieMap::generateScript()
{
    QString javascript;

    if(mapData.isEmpty())
        mapData = queryMetaData();

    if(!mapData.isEmpty())
    {

        javascript=       "require(["\
                "\"dojo/ready\","\
                "\"dojo/parser\","\
                "\"dojox/geo/openlayers/Map\","\
                "\"dojox/geo/openlayers/Layer\","\
                "\"dojox/geo/openlayers/GeometryFeature\","\
                "\"dojox/geo/openlayers/WidgetFeature\","\
                "\"dojox/gfx/fx\","\
                "\"dojox/charting/widget/Chart2D\","\
                "\"dojox/charting/themes/Charged\","\
                "\"dojox/charting/themes/Dollar"\
                "\"], function(ready) {"\
                "ready(function(){"\
                "console.log(dojo.version.major + \".\" + dojo.version.minor + \".\" + dojo.version.patch);"\
                "var map = new dojox.geo.openlayers.Map('map');"\
                "var layer = new dojox.geo.openlayers.Layer();";

        foreach(originMetaData omd, mapData)
        {
            javascript += "var " + omd.name + "= {latitude: " + QString::number(omd.latitude) + ", longitude: " + QString::number(omd.longitude) + "};"\
                    "var chartSize = 30;"\
                    "var piemap = {"\
                    "longitude : "+ omd.name+ ".longitude, latitude : "+omd.name+".latitude,"\
                    "createWidget : function(){"\
                    "var div = dojo.create(\"div\", {}, dojo.body());"\
                    "var chart = new dojox.charting.widget.Chart({"\
                    "margins : { l : 0, r : 0, t : 0, b : 0 }}, div);"\
                    "var c = chart.chart;"\
                    "c.addPlot(\"default\", {type : \"Pie\", radius :14, labels:false,font:\"normal normal normal 7pt Arial\", fontColor: \"white\","\
                    "labelOffset: 4});"\
                    "var ser =";
            int count=0;
            javascript += "(\"Series A\", [";
            foreach(QString base, omd.altCount.keys())
            {
                javascript += "{y: " + QString::number(omd.altCount[base]) + " }" ;
                count++;
                if(count<omd.altCount.size())
                {
                    javascript += ",";
                }
                else
                {
                    javascript += "]);";
                }
            }
            javascript += "c.addSeries(\"Series\", ser);"\
                    "c.setTheme(dojox.charting.themes.Charged);"\
                    "c.render();"\
                    "c.theme.plotarea.fill = undefined;"\
                    "return chart; "\
                    "},"\
                    "width : chartSize, height : chartSize"\
                    "};"\
                    "var graphFeature1 = new dojox.geo.openlayers.WidgetFeature(piemap);"\
                    "layer.addFeature(graphFeature1);";
        }
        javascript += "layer.redraw();"\
                "map.addLayer(layer);"\
                "map.zoomToMaxExtent();"\
                "map.fitTo({position : [" +centerlng +", "+ centerlat + "],extent : 35});"\
                "})"\
                "});";
    }
    return javascript;
}

QString DojoPieMap::drawBarChart()
{
    QString barScript="", labelScript="";

    if(mapData.isEmpty())
        mapData = queryMetaData();

    if(!mapData.isEmpty())
    {
        originMetaData omd = mapData.at(0);

        barScript = "<!DOCTYPE HTML>"\
                "<html lang=\"en\">"\
                "<head>"\
                "<title>Allele Distribution </title>"\
                "</head>"\
                "<body>"\
                "<div id=\"chartNode\" style=\"width:800px;height:250px;\"></div>"\
                "<!-- load dojo and provide config via data attribute -->"\
                "<!-- load dojo -->"\
                "<script src=\"http://ajax.googleapis.com/ajax/libs/dojo/1.7.1/dojo/dojo.js\"></script>"\
                "<script>"\
                "require([\"dojox/charting/Chart\",\"dojox/charting/themes/Charged\","\
                "\"dojox/charting/plot2d/StackedColumns\",\"dojox/charting/plot2d/Markers\","\
                "\"dojox/charting/axis2d/Default\",\"dojo/domReady!\""\
                "], function(Chart, theme) {";
        QMap<QString,QString> dataMap;
        foreach(QString allele, omd.altCount.keys())
        {
            QString data= "var chartData_"+allele+" = [";
            dataMap.insert(allele, data);
        }
        int count = 0;
        foreach(originMetaData omd, mapData)
        {
            int i=0;
            foreach(QString allele, omd.altCount.keys())
            {
                dataMap[allele] += QString::number(omd.altCount[allele]);
                i++;
            }
            labelScript+= "{ "\
                    "value: "+ QString::number(count+1) +", text: \""+ omd.name+ "\" }";
            if(count<mapData.size()-1)
            {
                foreach(QString allele, omd.altCount.keys()) dataMap[allele] += ",";
                labelScript += ",";
            }
            count++;
        }
        foreach(QString allele, omd.altCount.keys())
        {
            dataMap[allele] += "];";
            barScript += dataMap[allele];
        }
        barScript += "var chart = new Chart(\"chartNode\");"\
                "chart.setTheme(dojox.charting.themes.Charged);"\
                "chart.addPlot(\"default\", {"\
                "type: \"StackedColumns\","\
                "markers: true,"\
                "gap: 7,"\
                "label: true,"\
                "labelStyle: \"inside\""\
                "});"\
                "chart.addAxis(\"x\", {"\
                "labels: [" + labelScript + "], rotation:90});"\
                "chart.addAxis(\"y\", { title: \"Allele Count\", font: \"normal normal normal 8pt Tahoma\" , min:0, vertical: true, fixLower: \"major\", fixUpper: \"major\" });";
        //arranged in reverse order...
        QMapIterator<QString, int> i(omd.altCount);
        i.toBack();
        while (i.hasPrevious()) {
            i.previous();
            barScript+= "chart.addSeries(\"Chart Data "+ i.key()+" \",chartData_"+i.key()+");";
        }
        barScript+= "chart.render(); "\
                "});"\
                "</script>"\
                "</body>"\
                "</html>";
    }
    return barScript;
}

QList<originMetaData> DojoPieMap::queryMetaData()
{
    QList<originMetaData> omdList;
    QProgressDialog progressdialog(MainWindow::instance);
    progressdialog.setLabelText(tr("Fetching alleles from database.."));
    progressdialog.setModal(false);

    long ngroup;
    if(!dbq.isConnected()){
        emit databaseStatus("No connection to db ");
    }else
    {
        emit databaseStatus("Database connected");
        QSqlQuery qry, latqry;
        if(qry.exec("select count(*) from (select distinct place, latitude, longitude from samples, location where origin_id=location.id);"))
        {
            qry.next();
            ngroup = qry.value(0).toString().toLong();
        }
        progressdialog.setRange(0, ngroup);

        if(latqry.exec("select distinct location.place, location.latitude, location.longitude, base, count(base) as tot from variationtransaction,samples, location where samples.origin_id=location.id and variationtransaction.sample=samples.name and variationtransaction.position="+ QString::number(this->position) + " and variationtransaction.chrom=\'" +chrom +"\'  group by location.place, base order by location.place, base;"))
        {

            int value=0;
            progressdialog.show();
            if(latqry.next())
            {
                originMetaData omd;
                //take the first latitude and longitude as center of map
                centerlat = latqry.value(1).toString();
                centerlng = latqry.value(2).toString();

                //initializing omd.altCount, work on this later
                QStringList alleleList = dbq.queryAlleles(this->position);
                foreach(QString all, alleleList)
                    omd.altCount[all]=0;

                do
                {

                    progressdialog.setValue(value);
                    qApp->processEvents();
                    QString place,lat,lng;

                    double total, count, freq;
                    bool ok;

                    place = latqry.value(0).toString().replace(" ","_");
                    place = place.replace("-","_");
                    if (omd.name==place) //same with previous iteration
                    {
                        count= latqry.value(4).toString().toInt();
                        omd.altCount[latqry.value(3).toString()]=count;
                    }
                    else
                    {

                        if(!omd.name.isEmpty())
                            omdList.append(omd);

                        lat = latqry.value(1).toString();
                        lng = latqry.value(2).toString();

                        omd.name = place;
                        omd.latitude =lat.toDouble();
                        omd.longitude =lng.toDouble();

                        count= latqry.value(4).toString().toInt();
                        omd.altCount[latqry.value(3).toString()]=count;

                        value++;
                    }
                }while(latqry.next());
            }/*end of if(latqry.next())*/
            //emit databaseStatus("Database loaded" );

        }/*end of if starting query successfull*/
        else
            qDebug()<< latqry.lastError();
    }
    progressdialog.hide();

    if(omdList.isEmpty())// if it is still empty after the query
    {
        QMessageBox warning(QMessageBox::Critical, "Map Error ", "No data for selected position or malformed sample information .db file", QMessageBox::Ok);
        warning.exec();
    }
    return omdList;
}

QString DojoPieMap::generateHTML()
{
    QString jScript = generateScript();
    if(jScript.isEmpty()) return "";

    QString html = "<html><head><title>ACX</title><style>html, body {width: 100%;height: 100%;}</style>"\
            "<script type=\"text/javascript\" src=\"http://ajax.googleapis.com/ajax/libs/dojo/1.7.1/dojo/dojo.js\"></script>"\
            "<link rel=\"stylesheet\" href=\"http://ajax.googleapis.com/ajax/libs/dojo/1.7.1/dijit/themes/claro/claro.css\"/>"\
            "<link rel=\"stylesheet\" href=\"http://ajax.googleapis.com/ajax/libs/dojo/1.7.1/dijit/themes/claro/document.css\"/>"\
            "<script type=\"text/javascript\" src=\"http://openlayers.org/api/2.10/OpenLayers.js\"></script>"\
            "<script type=\"text/javascript\">";
    html.append(jScript);
    html.append("</script></head><body class=\"claro\"><div id=\"map\" style=\"width: 100%; height: 100%;\"></div></body></html>");

    return html;
}

int DojoPieMap::getPosition()
{
    return this->position;
}

