#ifndef jbrowsesettingdialog_H
#define jbrowsesettingdialog_H

#include <QDialog>
#include "mainwindow.h"

namespace Ui {
class JBrowseSettingDialog;
}

class JBrowseSettingDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit JBrowseSettingDialog(QWidget *parent = 0);
    ~JBrowseSettingDialog();
    
private slots:

    void on_pushButton_ok_clicked();

private:
    Ui::JBrowseSettingDialog *ui;
};

#endif // jbrowsesettingdialog_H

