/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Graphics Dojo project on Qt Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 or 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore>
#include <QtGui>
#include <QtWebKit>
#include "gmap.h"

#if QT_VERSION < 0x0040500
#error You need Qt 4.5 or newer
#endif

#define MAP_HTML "<html><head><script type=\"text/javascript\" "\
    "src=\"http://maps.google.com/maps/api/js?sensor=false\"></script>"\
    "<style>body {  font-family: sans-serif;  font-size: small;}"\
    "</style>"\
    "<script  type=\"text/javascript\">"\
    "function init(lat, lng, year) { "\
    " var map = new google.maps.Map(document.getElementById(\"map\"), "\
    "{ zoom: 2, center: new google.maps.LatLng(lat,lng), "\
    "mapTypeId: google.maps.MapTypeId.ROADMAP, optimized:false  });"\
    "var infoWindow = new google.maps.InfoWindow();"\
    " var layer = new google.maps.FusionTablesLayer({query: {"\
    "select: 'Isolation Place', from: '3881333', where: 'Year = ' +year}});"\
    "layer.setMap(map);"\
    "}"\
    "function updateSlider(lat, lng, year) { "\
        " var map = new google.maps.Map(document.getElementById(\"map\"), "\
        "{ zoom: 2, center: new google.maps.LatLng(lat,lng), "\
        "mapTypeId: google.maps.MapTypeId.ROADMAP, optimized:false  });"\
        " var layer = new google.maps.FusionTablesLayer({query: {"\
    "select: 'Isolation Place', from: '3881333', where: 'Year <=' + year}, styles:[{markerOptions: {iconName:\"large_green\"}}]});"\
        "layer.setMap(map);"\
        "}"\
    "function updateEqual(lat, lng, year) { "\
        " var map = new google.maps.Map(document.getElementById(\"map\"), "\
        "{ zoom: 2, center: new google.maps.LatLng(lat,lng), "\
        "mapTypeId: google.maps.MapTypeId.ROADMAP, optimized:false  });"\
        " var layer = new google.maps.FusionTablesLayer({query: {"\
    "select: 'Isolation Place', from: '3881333', where: 'Year = '+year}, styles:[{markerOptions: {iconName:\"large_green\"}}]});"\
        "layer.setMap(map);"\
        "}"\
    "</script>"\
    " </head><body style=\"margin:0px; padding:0px;\" onLoad=\"init(27,77.38)\">"\
    " <div id=\"map\" style=\"width: 100%; height: 100%;border: 1px solid black;margin-bottom: 10px;\"> </div>"\
    "</body></html>"


#define MAP_SQLITE "<html><head><script type=\"text/javascript\" "\
    "src=\"http://maps.google.com/maps/api/js?sensor=false\"></script>"\
    "<style>body {  font-family: sans-serif;  font-size: small;}"\
    "</style>"\
    "<script  type=\"text/javascript\">"\
    "function init(lat, lng, year) { "\
    " var map = new google.maps.Map(document.getElementById(\"map\"), "\
    "{ zoom: 2, center: new google.maps.LatLng(lat,lng), "\
    "mapTypeId: google.maps.MapTypeId.ROADMAP, optimized:false  });"\
    "}"\
    "function updateSlider(lat, lng, year) { "\
        " var map = new google.maps.Map(document.getElementById(\"map\"), "\
        "{ zoom: 2, center: new google.maps.LatLng(lat,lng), "\
        "mapTypeId: google.maps.MapTypeId.ROADMAP, optimized:false  });"\
        " var layer = new google.maps.FusionTablesLayer({query: {"\
    "select: 'Isolation Place', from: '3881333', where: 'Year <=' + year}, styles:[{markerOptions: {iconName:\"large_green\"}}]});"\
        "layer.setMap(map);"\
        "}"\
    "function updateEqual(lat, lng, year) { "\
        " var map = new google.maps.Map(document.getElementById(\"map\"), "\
        "{ zoom: 2, center: new google.maps.LatLng(lat,lng), "\
        "mapTypeId: google.maps.MapTypeId.ROADMAP, optimized:false  });"\
        " var layer = new google.maps.FusionTablesLayer({query: {"\
    "select: 'Isolation Place', from: '3881333', where: 'Year = '+year}, styles:[{markerOptions: {iconName:\"large_green\"}}]});"\
        "layer.setMap(map);"\
        "}"\
    "</script>"\
    " </head><body style=\"margin:0px; padding:0px;\" onLoad=\"init(27,77.38)\">"\
    " <div id=\"map\" style=\"width: 100%; height: 100%;border: 1px solid black;margin-bottom: 10px;\"> </div>"\
    "</body></html>"

GMap::GMap(QWidget *parent)
        : QWebView(parent)
    {
    this->setPage(new MapPage(this));
        QString content = MAP_HTML;
        QWebFrame *frame = page()->mainFrame();
        frame->setHtml(content);
    }

    void GMap::setCenter(qreal latitude, qreal longitude) {
        QString code = "map.set_center(new google.maps.LatLng(%1, %2));";
        QWebFrame *frame = page()->mainFrame();
        frame->evaluateJavaScript(code.arg(latitude).arg(longitude));
    }

    void GMap::loadMap()
    {
        this->setPage(new MapPage(this));
        QString content = MAP_HTML;
        QWebFrame *frame = page()->mainFrame();
        frame->setHtml(content);
    }

    void GMap::updateYear(int year)
    {
        QString code="updateEqual(27,77.38,"+QString::number(year)+")";
        QWebFrame *frame = page()->mainFrame();
        frame->evaluateJavaScript(code);
    }

    void GMap::updateYearsBefore(int currentYear)
    {
        QString code="updateSlider(27,77.38,"+QString::number(currentYear)+")";
        QWebFrame *frame = page()->mainFrame();
        frame->evaluateJavaScript(code);
    }
