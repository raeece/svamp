#include "jbrowsesettingdialog.h"
#include "ui_jbrowsesettingdialog.h"
#include <QFileDialog>

JBrowseSettingDialog::JBrowseSettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JBrowseSettingDialog)
{
    ui->setupUi(this);

    //read jbrowse file's path from svamp.temp

    QFile temp("svamp.temp");
    if(temp.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream fin(&temp);
        fin.readLine();//the first line is db path
        QString jbrowsePath = fin.readLine();
        QString lookseqPath = fin.readLine();

        ui->lineEdit_jbrowsefile->setText(jbrowsePath);
        ui->lineEdit_lookseq->setText(lookseqPath);
        temp.close();
    }

}

JBrowseSettingDialog::~JBrowseSettingDialog()
{
    delete ui;
}

void JBrowseSettingDialog::on_pushButton_ok_clicked()
{
    QString dbPath="";

    QFile temp("svamp.temp");
    if(temp.open(QIODevice::ReadOnly| QIODevice::Text))
    {
        QTextStream fin(&temp);
        dbPath= fin.readLine();//the first line is db path
        temp.close();
    }

    if(temp.open(QIODevice::WriteOnly| QIODevice::Text))
    {
        QTextStream fout(&temp);
        fout << dbPath << endl;
        fout << ui->lineEdit_jbrowsefile->text() << endl;
        fout << ui->lineEdit_lookseq->text()<< endl;
        temp.close();
    }

    close();
}
