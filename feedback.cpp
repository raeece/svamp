#include "feedback.h"
#include "ui_feedback.h"

Feedback::Feedback(QWidget *parent) :  QDialog(parent), ui(new Ui::Feedback) {
    ui->setupUi(this);
    setContextMenuPolicy(Qt::NoContextMenu);
    setWindowFlags(/*Qt::FramelessWindowHint*/Qt::WindowMinimizeButtonHint);
    //setAttribute(Qt::WA_TranslucentBackground);
    miType = 0;
}

Feedback::~Feedback() {
    delete ui;
}

void Feedback::initialise(int piType) {
    miType = piType;
    switch (miType) {
    case 0: {
        setWindowTitle("Loading data...");
        ui->label_1->setText("Sequence");
        ui->label_2->setText("Annotation");
        ui->label_3->setText("Variation");
        ui->label_4->setText("Statistics");
        break;
    }
    case 1: {
        setWindowTitle("Saving data...");
        ui->label_1->setText("Section");
        break;
    }
    }
    show();
}

void Feedback::update(int piSection, QString psMessage) {
    switch (miType) {
    case 0: {
        switch (piSection) {
        case 1: ui->label_5->setText(psMessage + " seqeunces"); break;
        case 2: ui->label_6->setText(psMessage + " annotations"); break;
        case 3: ui->label_7->setText(psMessage + " variants"); break;
        case 4: ui->label_8->setText("Calculating"); break;
        }
    }
    case 1: {
        switch (piSection) {
        case 1: ui->label_5->setText(psMessage); break;
        case 2: ui->label_6->setText(psMessage + " records"); break;
        }
    }
    }
    repaint();
    qApp->processEvents();
}

void Feedback::clear() {
    miType = -1;
    setWindowTitle("");
    ui->label_1->setText("");
    ui->label_2->setText("");
    ui->label_3->setText("");
    ui->label_4->setText("");
    ui->label_5->setText("");
    ui->label_6->setText("");
    ui->label_7->setText("");
    ui->label_8->setText("");
    close();
}
