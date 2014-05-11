#include "mapdatahandler.h"

#define LABEL_CLASS "function Label(opt_options) {"\
" this.setValues(opt_options);"\
"var span = this.span_ = document.createElement('span');"\
"span.style.cssText = 'position: relative; left: -50%; top: -8px; ' +"\
"   'white-space: nowrap; border: 1px solid blue; ' +"\
"'padding: 2px; background-color: white';"\
"var div = this.div_ = document.createElement('div');"\
"div.appendChild(span);"\
"div.style.cssText = 'position: absolute; display: none';"\
"};"\
"Label.prototype = new google.maps.OverlayView;"\
"Label.prototype.onAdd = function() {"\
" var pane = this.getPanes().overlayLayer;"\
" pane.appendChild(this.div_);"\
" var me = this;"\
"this.listeners_ = ["\
"  google.maps.event.addListener(this, 'position_changed',"\
" function() { me.draw(); }),"\
" google.maps.event.addListener(this, 'text_changed',"\
"function() { me.draw(); })"\
"];"\
"};"\
"Label.prototype.onRemove = function() {"\
" this.div_.parentNode.removeChild(this.div_);"\
" for (var i = 0, I = this.listeners_.length; i < I; ++i) {"\
"   google.maps.event.removeListener(this.listeners_[i]);"\
" }"\
"};"\
"Label.prototype.draw = function() {"\
" var projection = this.getProjection();"\
"var position = projection.fromLatLngToDivPixel(this.get('position'));"\
"var div = this.div_;"\
"div.style.left = position.x + 'px';"\
"div.style.top = position.y + 'px';"\
" div.style.display = 'block';"\
"this.span_.innerHTML = this.get('text').toString();"\
"};"

MapDataHandler::MapDataHandler()
{
   dbq.connect();
   if(dbq.isConnected()) dbConnected=true;
   else dbConnected = false;
   queryContent="";
}

MapDataHandler::~MapDataHandler()
{
    dbq.closeDB();
}

QString MapDataHandler::composeCode()
{
    QString html;
    QString labeljs= LABEL_CLASS;

    html = "<html><head><script type=\"text/javascript\" "\
            "src=\"http://maps.google.com/maps/api/js?v=3.1&sensor=true\"></script>"\
            "<script type=\"text/javascript\">"+ labeljs + "</script>"\
            "<style>body {  font-family: sans-serif;  font-size: small;}"\
            "</style>"\
            " </head><body style=\"margin:0px; padding:0px;\">"\
            " <div id=\"map\" style=\"width: 100%; height: 100%;border: 1px solid black;margin-bottom: 10px;\"> "\
            "<script type=\"text/javascript\">"\
            " var map = new google.maps.Map(document.getElementById(\"map\"), "\
            "{ zoom: 2, center: new google.maps.LatLng("+centerlat+","+centerlng+"), "\
            "mapTypeId: google.maps.MapTypeId.ROADMAP, optimized:false, navigationControl:true, draggable:true  });"\
            +queryContent+
            "</script>"\
            "</div>"\
            "</body></html>";
    return html;
}

void MapDataHandler::runQuery()
{
    if(!dbq.isConnected()){
        emit statusUpdate("No connection to db " );
    }else
    {
        QSqlQuery latqry;
        if(latqry.exec("select distinct place, latitude, longitude from samples, location where origin_id=location.id;"))
        {
            if(latqry.next())
            {
                //take the first latitude and longitude as center of map
                centerlat = latqry.value(1).toString();
                centerlng = latqry.value(2).toString();
              do
              {
                    QMap<QString, double> alt;//store the alternate bases, to be displayed with the allele frequency
                    QString place,lat,lng;

                    double total;

                    place = latqry.value(0).toString().replace(" ","_");
                    QString nameNoDash = place;
                    nameNoDash = nameNoDash.replace("-","_");

                    lat = latqry.value(1).toString();
                    lng = latqry.value(2).toString();

                    queryContent.append("var marker"+nameNoDash+" = new google.maps.Marker({"\
                                        "position: new google.maps.LatLng("+lat+","+lng+"),"\
                                        "map: map");
                    queryContent.append("});  ");                  
                        queryContent.append(addFullInfoWindow(place));

                    total=0;
               }while(latqry.next());
            }/*end of if(latqry.next())*/
             emit statusUpdate("Database loaded" );
        }/*end of if starting query successfull*/
    }
}

void MapDataHandler::updateQuery(QString condition)
{
    queryContent = "";
    if(!dbq.isConnected()){
        emit statusUpdate("No connection to db :( " );
    }else
    {
        QSqlQuery latqry;
        if(latqry.exec("select distinct place, latitude, longitude from samples, location where origin_id=location.id and "+ condition + " ;"))
        {
            if(latqry.next())
            {
                //take the first latitude and longitude as center of map
                centerlat = latqry.value(1).toString();
                centerlng = latqry.value(2).toString();
              do
              {
                    QMap<QString, double> alt; //store the alternate bases, to be displayed with the allele frequency
                    QString place,lat,lng;

                    double total, freq;

                    place = latqry.value(0).toString().replace(" ","_");
                    lat = latqry.value(1).toString();
                    lng = latqry.value(2).toString();

                    //placing the marker
                    queryContent.append("var marker"+place+" = new google.maps.Marker({"\
                                        "position: new google.maps.LatLng("+lat+","+lng+"),"\
                                        "map: map,"\
                                        "title:\' ");

                    //attaching the alleles' frequency to title
                    for(int c=0;c<alt.size();c++)
                    {
                        QString key= alt.keys().at(c);
                        freq= alt[key]/total*100;
                        queryContent.append(key+": "+QString::number(freq,'f',1)+"_");
                    }
                    queryContent.append("\'});  ");

                    queryContent.append(addFullInfoWindow(place, condition));

                    total=0;
               }while(latqry.next());
            }/*end of if(latqry.next())*/
        }/*end of if starting query successfull*/

    }
}

QStringList MapDataHandler::queryYearRange()
{
    QStringList year;
    if(!dbq.isConnected()){
        emit statusUpdate("No connection to db :( " );
    }else
    {
        QSqlQuery qry;
        if(qry.exec("select min(year) from samples;"))
        {
            if(qry.next())
            {
                  year.append(qry.value(0).toString());
            }
        }else
            year.append("0");

        if(qry.exec("select max(year) from samples;"))
        {
            if(qry.next())
            {
                  year.append(qry.value(0).toString());
            }
        }else
            year.append("3000");
    }
    return year;
}

QString MapDataHandler::addInfoWindow(QString place)
{
    QSqlQuery sampleqry;
    QString script="";
    script.append("var infowin"+place+"= new google.maps.InfoWindow();");
    if(sampleqry.exec("select Name from Samples where Origin=\'"+place+"\';"))
    {
        if(sampleqry.next())
        {
            QString placeList="";
            do
            {
                placeList += sampleqry.value(0).toString()+"<br>";
            }while(sampleqry.next());
            script.append("infowin"+place+".setContent(\'"+placeList+"\');");
        }
    }
    else
    {
        script.append("infowin"+place+".setContent('No Information Available');");
    }
    script.append("google.maps.event.addListener(marker"+place+", 'click', function() {"\
                  "infowin"+place+".open(map,marker"+place+");"\
                  "});");
    return script;
}

QString MapDataHandler::addFullInfoWindow(QString place)//at the moment it just include name of samples and their isolation year, more information can be included (to be discussed)
{
    QSqlQuery sampleqry;
    QString script="";
    QString placeName = place;
    place = place.replace("-","_");
    script.append("var infowin"+place+"= new google.maps.InfoWindow();");
    if(sampleqry.exec("select name,year from samples,location where location.place=\'"+placeName.replace("_"," ") +"\' and origin_id=location.id ;"))
    {
        if(sampleqry.next())
        {
            QString placeList="";
            do
            {
                placeList += sampleqry.value(0).toString()+" ( "+sampleqry.value(1).toString()+") <br>"; // append name of samples and their isolation year
            }while(sampleqry.next());
            script.append("infowin"+place+".setContent(\'"+placeList+"\');");
        }else
            qDebug() << "No result for query: "<< sampleqry.lastQuery();
    }
    else
    {
        script.append("infowin"+place+".setContent('No Information Available');");
    }
    script.append("google.maps.event.addListener(marker"+place+", 'click', function() {"\
                  "infowin"+place+".open(map,marker"+place+");"\
                  "});");
    return script;
}

QString MapDataHandler::addFullInfoWindow(QString place, QString condition)
{
    QSqlQuery sampleqry;
    QString placeName = place;
    QString script="";
    place = place.replace("-","_");
    script.append("var infowin"+place+"= new google.maps.InfoWindow();");
    if(sampleqry.exec("select name,year from samples,location where location.place=\'"+placeName.replace("_"," ") +"\' and origin_id=location.id  and "+ condition+ ";"))
    {
        if(sampleqry.next())
        {
            QString placeList="";
            do
            {
                placeList += sampleqry.value(0).toString()+" ( "+sampleqry.value(1).toString()+") <br>"; // append name of samples and their isolation year
            }while(sampleqry.next());
            //qDebug() << "placeList : " << placeList;
            script.append("infowin"+place+".setContent(\'"+placeList+"\');");
        }
    }
    else
    {
        script.append("infowin"+place+".setContent('No Information Available');");
    }
    script.append("google.maps.event.addListener(marker"+place+", 'click', function() {"\
                  "infowin"+place+".open(map,marker"+place+");"\
                  "});");
    //qDebug() << "infowin"+place+": " << script;
    return script;
}
