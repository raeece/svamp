#include "colors.h"
#include "mainwindow.h"
#include "ui_colors.h"

Colors::Colors(QWidget *parent) : QDialog(parent), ui(new Ui::Colors) {
    ui->setupUi(this);
    ui->comboBox_1->addItems(QColor::colorNames());
    ui->comboBox_2->addItems(QColor::colorNames());
    ui->comboBox_3->addItems(QColor::colorNames());
    ui->comboBox_4->addItems(QColor::colorNames());
    ui->comboBox_5->addItems(QColor::colorNames());
    ui->comboBox_6->addItems(QColor::colorNames());
    ui->comboBox_1->setCurrentIndex(QColor::colorNames().indexOf(MainWindow::instance->colors[0]));
    ui->comboBox_2->setCurrentIndex(QColor::colorNames().indexOf(MainWindow::instance->colors[1]));
    ui->comboBox_3->setCurrentIndex(QColor::colorNames().indexOf(MainWindow::instance->colors[2]));
    ui->comboBox_4->setCurrentIndex(QColor::colorNames().indexOf(MainWindow::instance->colors[3]));
    ui->comboBox_5->setCurrentIndex(QColor::colorNames().indexOf(MainWindow::instance->colors[4]));
    ui->comboBox_6->setCurrentIndex(QColor::colorNames().indexOf(MainWindow::instance->colors[5]));
}

Colors::~Colors() {
    delete ui;
}

void Colors::on_pushButton_clicked() {
    MainWindow::instance->colors[0] = ui->comboBox_1->currentText();
    MainWindow::instance->colors[1] = ui->comboBox_2->currentText();
    MainWindow::instance->colors[2] = ui->comboBox_3->currentText();
    MainWindow::instance->colors[3] = ui->comboBox_4->currentText();
    MainWindow::instance->colors[4] = ui->comboBox_5->currentText();
    MainWindow::instance->colors[5] = ui->comboBox_6->currentText();
    MainWindow::instance->updateLegend();

    QFile file("colors.pv");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream fout(&file);
        fout << ui->comboBox_1->currentText() << endl;
        fout << ui->comboBox_2->currentText() << endl;
        fout << ui->comboBox_3->currentText() << endl;
        fout << ui->comboBox_4->currentText() << endl;
        fout << ui->comboBox_5->currentText() << endl;
        fout << ui->comboBox_6->currentText() << endl;
        file.close();
    }else
        qDebug() << "colors.pv file not found";
    close();
}
