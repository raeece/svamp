#include <QDebug>
#include "log.h"
#include "ui_log.h"

Log::Log(QWidget *parent) : QDialog(parent), ui(new Ui::Log) {
    ui->setupUi(this);
    setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint | Qt::WindowStaysOnTopHint));
    hide();
}

Log::~Log() {
    delete ui;
}

void Log::outputMessage(QtMsgType type, const QString &msg) {
    switch (type) {
    case QtDebugMsg:    ui->logBrowser->append(msg); break;
    case QtWarningMsg:  ui->logBrowser->append(tr("-- WARNING:  %1").arg(msg)); break;
    case QtCriticalMsg: ui->logBrowser->append(tr("-- CRITICAL: %1").arg(msg)); break;
    case QtFatalMsg:    ui->logBrowser->append(tr("-- FATAL:    %1").arg(msg)); break;
    }
}
