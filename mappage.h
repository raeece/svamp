#ifndef MAPPAGE_H
#define MAPPAGE_H

#include <QCoreApplication>
#include <QWebPage>

class MapPage : public QWebPage
{
    Q_OBJECT
public:
    explicit MapPage(QObject *parent = 0);
    ~MapPage();

private:
    void javaScriptConsoleMessage(const QString &message, int lineNumber, const QString &sourceID);
};

#endif // MAPPAGE_H
