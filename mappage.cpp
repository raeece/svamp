#include "mappage.h"
#include <QDebug>

MapPage::MapPage(QObject *parent) : QWebPage(parent)
{
    QCoreApplication* theApplication = QCoreApplication::instance();
    theApplication->setApplicationName( "chrome" );
    theApplication->setApplicationVersion( "1.0" );
}

MapPage::~MapPage()
{
}

void MapPage::javaScriptConsoleMessage(const QString &message, int lineNumber, const QString &sourceID)
{
    qDebug() << message << lineNumber << sourceID;
}
