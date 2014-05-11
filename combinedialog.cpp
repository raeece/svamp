#include "combinedialog.h"
#include "ui_combinedialog.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

CombineDialog::CombineDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CombineDialog)
{
    ui->setupUi(this);
    area = ((MainWindow*)parent)->getUI()->tracks;
    seq = area->getSequence();

    QList <QString> chrlist;
    chrlist = seq->chromosomes.keys();
    ui->chromComboBox->clear();
    ui->chromComboBox->addItems(chrlist);// filled the chromosome combo boc

    layoutCount=1;
    lineEditList.append(ui->fileLineEdit_1); //append one default lineEdit to the list
    pushButtonList.append(ui->browseButton_1);//append corresponding pushButton to the list
}

CombineDialog::~CombineDialog()
{
    delete ui;
}

void CombineDialog::on_browseButton_1_clicked()
{
    QString f = QFileDialog::getOpenFileName (this, tr("Select local file"), ui->fileLineEdit_1->text(), tr("BED Files (*.bed)"));
    if (!f.isNull()) ui->fileLineEdit_1->setText(f);
}

void CombineDialog::on_buttonBox_accepted()
{
    close();
    int start=time(NULL);//start the timer

    QString args[layoutCount+2];// length of array depends on how many lineEdit the dialog will have
    if (ui->nameLineEdit->text()=="")
        args[0]=ui->chromComboBox->currentText().append(".cov"); // if the user doesn't specify the name for the output file then by default chromosome name will be used
    else
        args[0]=ui->nameLineEdit->text().append(".cov");

    args[1]=QString::number(layoutCount);//convert integer to QString

    for (int x=0;x<layoutCount;x++)
        args[x+2] = lineEditList[x]->text();

    MainWindow::instance->createCoverFile(args);

    int end=time(NULL);
    int elapsed= end-start;
    ///qDebug() << "MainWindow::Merging finished, execution took " << elapsed << " seconds";
}

void CombineDialog::on_addButton_clicked()
{
    QLineEdit *lineEdit = new QLineEdit(this); //create new lineEdit
    QPushButton *pushButton = new QPushButton(this); // create new Button to browse the file
    lineEditList.append(lineEdit);
    pushButtonList.append(pushButton);
    pushButton->setText("Browse");
    QString qStr = QString::number(layoutCount);
    pushButton->setObjectName(qStr);// object variable name here will be used as numbering in order to know in which lineEdit the file name will be put
    ui->gridLayout->addWidget( pushButton,layoutCount,1);
    ui->gridLayout->addWidget( lineEdit,layoutCount,0 );
    connect( pushButton, SIGNAL(clicked()), this, SLOT(getFile()));// give link to the "getFile" function
    layoutCount++;

}

void CombineDialog::getFile()
{
    // cast the sender to QPushButton to know which button is clicked
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    int index= button->objectName().toInt();//get the button iddentifier

    QString f = QFileDialog::getOpenFileName (this, tr("Select local file"),lineEditList[index]->text(), tr("BED Files (*.bed)"));
    if (!f.isNull()) lineEditList[index]->setText(f);//based on buttonID, it will placed filename on corresponding lineEdit
}
