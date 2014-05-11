/*
 *  SVAMP - Sequence Variation Analysis , Map, and Phylogeny
 *
 *  www.cbrc.kaust.edu.sa/svamp
 *
 *  (c) Lailatul Hidayah, RaeeceNaeem Ghazali
 *
 *  This software was written, in large part, at King Abdullah University of Science and Technology (KAUST)
 *
 *  This file is part of SVAMP.
 *
 *  SVAMP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SVAMP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SVAMP.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "searchresult.h"
#include "annotation.h"
#include "colors.h"
#include "groupmanager.h"
#include "save.h"
#include "sourcesdialog.h"
#include "trackgraph.h"
#include "trackgraphgc.h"
#include "trackgraphvd.h"
#include "trackgraphfst.h"
#include "trackgraphtd.h"
#include "basecoveragedialog.h"
#include "combinedialog.h"
#include "pcoa.h"
#include "pcoadialog.h"
#include "ui_pcoadialog.h"
#include "mapdialog.h"
#include "treeoptionsdialog.h"
#include "mapalleledialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QCheckBox>
#include <QDir>
#include <QList>
#include <QTextStream>

QPointer<Log> logBrowser;

void logOutput(QtMsgType type, const char *msg) {
    if (logBrowser) logBrowser->outputMessage(type, msg);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    logBrowser = new Log;
    qInstallMsgHandler(logOutput);

    clickedPosition=0;
    createMenu();

    QDesktopWidget desktop;

    totalSNP =0;
    visible_range = 200;
    props_are_sticky = false;
    adjusting = false;
    instance = this;
    ui->setupUi(this);

    ui->zoomSlider->setRange(1,1);
    ui->lineSize->setText("20");
    ui->browserBox->setEnabled(false);
    groups["Ungrouped"] = ui->ungroupedCheck;

    setMaximumSize(desktop.availableGeometry().width(), desktop.availableGeometry().height());
    showMaximized();
    resize(desktop.availableGeometry().width(), desktop.availableGeometry().height());

    setMacUp();

    QFile file("colors.pv");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream fin(&file);
        colors[0] = fin.readLine();
        colors[1] = fin.readLine();
        colors[2] = fin.readLine();
        colors[3] = fin.readLine();
        colors[4] = fin.readLine();
        colors[5] = fin.readLine();
        file.close();
    } else {
        colors[0] = "darkblue";
        colors[1] = "green";
        colors[2] = "yellow";
        colors[3] = "red";
        colors[4] = "black";
        colors[5] = "lightgray";
    }

    //change the groupwidget frame to be scrollable
    QPointer<QScrollArea> scroll = new QScrollArea(this);
    scroll->setWidget(ui->groupsWidget);
    ui->groupsWidget->resize(200,75);
    ui->scrollVerticalLayout->addWidget(scroll);
    //ui->groupWidget_verticalLayout->addWidget(scroll);

    updateLegend();
    updateCounts(0,0,0,0,0);

    disableDataMenu();
}

MainWindow::~MainWindow() {
    DBQuery dbq;

    delete ui;

    clearMenuPointers();

}
void MainWindow::setMacUp() {
    // Application icon
    setWindowIcon(QIcon(":/images/svamp.png"));
    //ui->aboutButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    //ui->loadButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    //ui->saveButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    //ui->manageGroupsButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    //ui->colorButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->findButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->findClearButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->rsdButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->settingsButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->positionRightButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->positionLeftButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->goButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    ui->commonCheckBox->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->nonCodingCheckBox->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->synonymousCheckBox->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->nonSynonymousCheckBox->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->indelCheckBox->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    ui->chrBox->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    ui->lineFrom->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->lineTo->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->lineSize->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->lineQuality->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->lineDepth->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->findQuery->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    ui->positionSlider->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->browserSizeSlider->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->qualitySlider->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->readDepthSlider->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->zoomSlider->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    ui->readDepthLabel->setAttribute(Qt::WA_LayoutUsesWidgetRect);
}

void MainWindow::keyPressEvent(QKeyEvent* key) {
    if (Qt::ControlModifier == key->modifiers()) {
        switch (key->key()) {
        case Qt::Key_C: editColour(); break;
        case Qt::Key_G: viewGroupManager(); break;
        case Qt::Key_H: viewAboutPage(); break;
        case Qt::Key_L: loadFilesAction(); break;
        case Qt::Key_S: saveAction(); break;
        }
    }
}

void MainWindow::exportView(QString filename)
{

    QPixmap sampleListPM = QPixmap::grabWidget(ui->listWidget);
    QPixmap tracksPM = QPixmap::grabWidget(ui->tracks);
    if (sampleListPM.isNull() || tracksPM.isNull())
        qDebug() << "Can't grab widget for exporting to picture. One or more item(s) is blank";

    QImage sampleListIm = sampleListPM.toImage();
    QImage tracksIm = tracksPM.toImage();

    QPainter p;
    QRect rList (4,this->top+1,this->label_offset-8,this->height);

    int tracksWidth = tracksPM.width();
    int tracksHeight = tracksPM.height();

    QImage imContainer(tracksWidth, tracksHeight,QImage::Format_RGB32 );
    imContainer.fill(Qt::white);

    p.begin(&imContainer);
    p.drawImage(0,0,tracksIm);
    p.drawImage(rList.topLeft(),sampleListIm);

    p.end();

    if(imContainer.save(filename)) qDebug() << "Picture saved";
}

void MainWindow::outOfMemory(QString msg) {
    QMessageBox msgBox;
    msgBox.setText("Out of memory - program exiting: " + msg);
    msgBox.exec();
    exit(-1);
}

bool MainWindow::isCommon(){
    return ui->commonCheckBox->isChecked();
}

bool MainWindow::isNonCoding() {
    return ui->nonCodingCheckBox->isChecked();
}

bool MainWindow::isSynonymous() {
    return ui->synonymousCheckBox->isChecked();
}

bool MainWindow::isNonSynonymous() {
    return ui->nonSynonymousCheckBox->isChecked();
}

bool MainWindow::isIndel() {
    return ui->indelCheckBox->isChecked();
}

bool MainWindow::isGroup(QString group) {
    if (groups.contains(group)) return groups[group]->isChecked();
    return false;
}

void MainWindow::updateCounts(int nonCoding, int nonSynonymous, int synonymous, int indel, int common) {
    ui->nonCodingLabel->setText(QString::number(nonCoding));
    ui->nonSynonymousLabel->setText(QString::number(nonSynonymous));
    ui->synonymousLabel->setText(QString::number(synonymous));
    ui->indelLabel->setText(QString::number(indel));
    ui->commonLabel->setText(QString::number(common));
}

void MainWindow::updateLegend() {
    ui->labelLegend->setText("<font color='" + colors[0] + "'>00</font> - <font color='" + colors[1] + "'>01</font> - <font color='" + colors[2] + "'>10</font> - <font color='" + colors[3] + "'>11</font> - <font color='" + colors[4] + "'>Indel</font> - <font color='" + colors[5] + "'>Missing</font>");
}

void MainWindow::loadFilesAction() {
    SourcesDialog sd(this);
    QString seq = "", anno = "", var = "", filter = "", db="";

    QFile file("last.pv");

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream fin(&file);
        seq = fin.readLine();
        anno = fin.readLine();
        var = fin.readLine();
        db=fin.readLine();
        filter = fin.readLine();
        file.close();
    }

    for (int a = 0; a < ui->tracks->tracklist.size(); a++) {
        Track *t = ui->tracks->tracklist[a];
        if (t->whatsthis() == "sequence") seq = ((TrackSequence*)t)->seq.source_file;
        if (t->whatsthis() == "annotation") anno = ((TrackAnnotation*)t)->data.source_file;
        if (t->whatsthis() == "variation") var = ((TrackVariation*)t)->variants.source_file;
        if (t->whatsthis() == "variation") filter = ((TrackVariation*)t)->variants.filter;
    }

    sd.initialise(seq,anno,var,filter,db);
    sd.setModal(true);
    sd.exec();
}

void MainWindow::loadData(QString s, QString a, QString v, QString f, QString d) {
    qDebug()<< QDateTime::currentDateTime().toString("yyyy-mm-dd hh:mm:ss")<< "MainWindow::loadData start";
    ui->statusBar->showMessage("Loading data...");

    loading = true;
    feedback.initialise(0);
    ui->settingsBox->setEnabled(false);
    ui->browserBox->setEnabled(false);

    groups.clear();
    groups["Ungrouped"] = ui->ungroupedCheck;
    ui->groupsWidget->setMinimumHeight(21 * groups.count());
    ui->tracks->tracklist.clear();

    //remove all checkboxes except ungrouped
    QObjectList checkBoxList = ui->groupsWidget->children();
    foreach(QObject * cb, checkBoxList)
    {
        QString cbText = ((QCheckBox*)cb)->text();
        if(cbText!="Ungrouped")
            ((QCheckBox*)cb)->hide();
    }

    TrackSequence *seq = new TrackSequence(this);
    TrackAnnotation *ann = new TrackAnnotation(this);
    TrackVariation *var = new TrackVariation(this);
    TrackGraphGC *gcGraph = new TrackGraphGC(this);
    TrackGraphVD *vdGraph = new TrackGraphVD(this);
    TrackGraphFST *fstGraph = new TrackGraphFST(this);
    TrackGraphTD *tdGraph = new TrackGraphTD(this);

    ui->statusBar->showMessage("Loading sequence data...");
    if (! seq->seq.loadFASTA(s, f)) {
        QMessageBox msgBox;
        msgBox.setText("No data after filtering on " + f + "\n" + "or\n" + "Bad sequence file: " + s);
        msgBox.exec();
        feedback.clear();
        return;
    }

    ui->statusBar->showMessage("Loading annotation data...");
    if (! ann->data.loadGFF3(a, &seq->seq, f)) {
        QMessageBox msgBox;
        msgBox.setText("Missing annotation file: " + a);
        msgBox.exec();
        feedback.clear();
        return;
    }

    ui->statusBar->showMessage("Loading variation data...");

    if (! var->variants.loadVCF(v, &seq->seq, &ann->data, f)) {
        QMessageBox msgBox;
        msgBox.setText("Missing or badly formatted variant file: " + v);
        msgBox.exec();
        feedback.clear();
        return;
    }

    feedback.update(4);
    ui->tracks->tracklist.append(seq);
    ui->tracks->tracklist.append(ann);
    ui->tracks->tracklist.append(var);
    ui->tracks->tracklist.append(gcGraph);
    ui->tracks->tracklist.append(vdGraph);
    ui->tracks->tracklist.append(fstGraph);
    ui->tracks->tracklist.append(tdGraph);

    // add checkbox for each track
    for (int i=1;i<=7;i++){
    QCheckBox *trackcheckbox = new QCheckBox("",ui->tracks);
    trackcheckbox->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    trackcheckbox->setStyleSheet("background-color: rgb(110,152,184);border-color: beige");
    trackcheckbox->setVisible(true);
    trackcheckbox->setEnabled(true);

    // disable graph tracks for faster navigation
    if(i<=3){
    trackcheckbox->setChecked(true);
    }
    else{
    trackcheckbox->setChecked(false);
    }
    connect(trackcheckbox, SIGNAL(clicked()),
            this, SLOT(track_checkbox_clicked()));
    ui->tracks->tracklistcheckbox.append(trackcheckbox);
    }

    ui->statusBar->showMessage("Calculating GC density metric...");
    gcGraph->calculateData();

    ui->statusBar->showMessage("Calculating relative variant density metric...");
    vdGraph->calculateData();

    ui->statusBar->showMessage("Calculating Fst metric...");
    fstGraph->calculateData();

    ui->statusBar->showMessage("Calculating Tajima's D metric...");
    tdGraph->calculateData();

    QFile file("last.pv");

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream fout(&file);
        fout << s << endl << a << endl << v << endl << d << endl << f << endl;
        file.close();
    }

    ui->statusBar->showMessage("Updating UI...");
    QList <QString> chrlist;
    chrlist = seq->seq.chromosomes.keys();
    ui->chrBox->clear();
    ui->chrBox->addItems(chrlist);
    loading = false;

    on_chrBox_currentIndexChanged(chrlist[0]);
    ui->browserSizeSlider->setValue(ui->browserSizeSlider->value());
    on_findClearButton_clicked();
    stickyProps(false);
    ui->statusBar->clearMessage();
    ui->settingsBox->setEnabled(true);
    ui->browserBox->setEnabled(true);

    feedback.clear();

    enableDataMenu();

    //apply initial grouping
    applyInitialGrouping();

    this->height = seq->height;
    this->label_offset = var->label_offset;
    this->top = var->top;

    showDetails();

    qDebug()<< QDateTime::currentDateTime().toString("yyyy-mm-dd hh:mm:ss")<< "MainWindow::loadData end";

}

void MainWindow::changeEvent(QEvent *e) {
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_findQuery_returnPressed() {
    on_findButton_clicked();
}

void MainWindow::on_findButton_clicked() {
    on_findClearButton_clicked();

    QString query = ui->findQuery->text();
    if (query.isEmpty()) return;

    for (int a = 0; a < ui->tracks->tracklist.size(); a++) {
        ui->tracks->tracklist[a]->runSearch(query,results);
    }

    for (int a = 0; a < results.size(); a++) {
        ui->findResults->addItem(results[a]->displayname);
    }

    ui->findResults->show();
}

void MainWindow::on_findClearButton_clicked() {
    for (int a = 0; a < results.size(); a++) delete results[a];
    results.clear();
    ui->findResults->clear();
}

void MainWindow::on_findResults_currentRowChanged(int currentRow) {
    if (currentRow == -1) return;
    SearchResult *sr = results[currentRow];
    long w = sr->to - sr->from;
    setRange(sr->chr, sr->from - w/20, sr->to + w/20);
    if (sr->annotation_item) showProps (sr->annotation_item->getDescription());
}

void MainWindow::showProps (QString s) {
    if (props_are_sticky) return;
    if (last_props == s) return;
    last_props = s;

    s.replace("<h2>","<div style='padding:1px;font-size:14pt;font-weight:bold;background-color:#A5DBEB'>");
    s.replace("</h2>","</div>");
    s = "<html><body style='font-size:14pt; font:Arial'>" + s + "</body></html>";
    ui->props->setHtml(s);
}

void MainWindow::stickyProps (bool sticky) {
    if (last_props == "") sticky = false;
    if (props_are_sticky == sticky) return; // Nothing to do
    if (sticky) {
        showProps (last_props);
        ui->rsdButton->show();
        props_are_sticky = sticky;
    } else {
        props_are_sticky = sticky;
        showProps ("");
        ui->rsdButton->hide();
    }
}

void MainWindow::on_rsdButton_clicked() {
    stickyProps(false);
}

void MainWindow::setRange (QString chr, long from, long to, long padding) {
    adjusting = true;

    // Chromosome change, if any
    if (chr != ui->tracks->range.chromosome) {
        int idx = ui->chrBox->findText(chr,Qt::MatchExactly);
        if (idx < 0) return;
        ui->chrBox->setCurrentIndex(idx);
    }

    // Position change
    from -= padding;
    to += padding;
    if (from < 1) {
        to += -from;
        from = 1;
    }
    if (to < ui->tracks->posPerWidth()) to = ui->tracks->posPerWidth();

    Sequence *seq = ui->tracks->getSequence();
    if (seq && to > seq->chromosomes[chr].length()) to = seq->chromosomes[chr].length();

    ui->tracks->setRange(chr, from, to);
    ui->zoomSlider->setValue(to-from);
    ui->positionSlider->setValue(from);
    ui->lineFrom->setText(QString::number(from));
    ui->lineTo->setText(QString::number(to));
    ui->tracks->changeChromosome();
    ui->tracks->checkAndPaint();
    adjusting = false;
}

void MainWindow::on_chrBox_textChanged(QString currentText) {

    on_chrBox_currentIndexChanged(currentText);

}

void MainWindow::on_chrBox_currentIndexChanged(QString currentText) {

    Sequence *seq = ui->tracks->getSequence();
    Variation *var = ui->tracks->getVariation();

    if (loading) return;
    if (!seq) return;
    if (!var) return;

    adjusting = true;
    ui->tracks->delayPaint = true;
    ui->statusBar->showMessage("Changing chromosome...");

    long chrlen = seq->chromosomes[currentText].length();
    if (chrlen <= 1 )
    {
        QMessageBox msgBox;
        msgBox.setText("No sequence found or corrupted fasta file");
        msgBox.exec();
    }
    Extrema extrema = var->extrema[currentText];
    visible_range = chrlen;

    ui->tracks->setRange(currentText, 1, chrlen);
    ui->lineFrom->setText(QString::number(1));
    ui->lineTo->setText(QString::number(chrlen));
    ui->zoomSlider->setRange(100, chrlen);
    ui->zoomSlider->setValue(std::min((long)1000,chrlen));

    ui->qualitySlider->setRange(extrema.qualityMin, extrema.qualityMin < extrema.qualityMax ? extrema.qualityMax : extrema.qualityMin);
    ui->qualitySlider->setValue(extrema.qualityMin);
    ui->qualitySlider->setEnabled(ui->qualitySlider->maximum() - ui->qualitySlider->minimum() > 0);
    ui->lineQuality->setText(QString::number(ui->qualitySlider->minimum()));
    ui->lineQuality->setEnabled(ui->qualitySlider->maximum() - ui->qualitySlider->minimum() > 0);

    ui->readDepthSlider->setRange(extrema.readDepthMin, extrema.readDepthMin < extrema.readDepthMax ? extrema.readDepthMax : extrema.readDepthMin);
    ui->readDepthSlider->setValue(extrema.readDepthMin);
    ui->readDepthSlider->setEnabled(ui->readDepthSlider->maximum() - ui->readDepthSlider->minimum() > 0);
    ui->lineDepth->setText(QString::number(ui->readDepthSlider->minimum()));
    ui->lineDepth->setEnabled(ui->readDepthSlider->maximum() - ui->readDepthSlider->minimum() > 0);

    on_positionSlider_valueChanged(1);

    ui->tracks->delayPaint = false;
    ui->tracks->changeChromosome();
    ui->tracks->checkAndPaint();
    adjusting = false;

    ui->statusBar->clearMessage();
    showDetails();
//    qDebug() << "MainWindow::on_chrBox_currentIndexChanged end";
}

void MainWindow::on_lineFrom_returnPressed() {
    on_goButton_clicked();
}

void MainWindow::on_lineTo_returnPressed() {
    on_goButton_clicked();
}

void MainWindow::on_goButton_clicked() {
    setRange(ui->tracks->range.chromosome, ui->lineFrom->text().toLong(), ui->lineTo->text().toLong());
}

void MainWindow::on_qualitySlider_valueChanged(int value) {
//    qDebug() << "MainWindow::on_qualitySlider_valueChanged";
    ui->lineQuality->setText(QString::number(value));
    ui->tracks->setQuality(value);
    ui->tracks->changeFilter();
    if (adjusting) return;
    ui->tracks->checkAndPaint();
}

void MainWindow::on_readDepthSlider_valueChanged(int value) {
//    qDebug() << "MainWindow::on_readDepthSlider_valueChanged";
    ui->lineDepth->setText(QString::number(value));
    ui->tracks->setReadDepth(value);
    ui->tracks->changeFilter();
    if (adjusting) return;
    ui->tracks->checkAndPaint();
}

void MainWindow::on_settingsButton_clicked() {
    if ("<" == ui->settingsButton->text()) {
        ui->settingsBox->hide();
        ui->settingsButton->setText(">");
        ui->settingsButton->setToolTip("Show settings");
        ui->tracks->recalculateLayout();
        ui->tracks->checkAndPaint();
    } else {
        ui->settingsBox->show();
        ui->settingsButton->setText("<");
        ui->settingsButton->setToolTip("Hide settings");
        ui->tracks->recalculateLayout();
        ui->tracks->checkAndPaint();
    }
}

void MainWindow::on_positionSlider_valueChanged(int value) {
//    qDebug() << "MainWindow::on_positionSlider_valueChanged start " << value << " " << ui->zoomSlider->value();
    ui->tracks->setRange(ui->tracks->range.chromosome, value, value + ui->zoomSlider->value() - 1);
    ui->lineFrom->setText(QString::number(value));
    ui->lineTo->setText(QString::number(value + ui->zoomSlider->value()));
    ui->tracks->changePosition();
    if (adjusting) return;
    ui->tracks->checkAndPaint();
//    qDebug() << "MainWindow::on_positionSlider_valueChanged end";
}

void MainWindow::on_zoomSlider_valueChanged(int value) {
//    qDebug() << "MainWindow::on_zoomSlider_valueChanged start";
    Q_UNUSED(value)
    Sequence *seq = ui->tracks->getSequence();
    if (!seq) return;
    visible_range = ui->zoomSlider->value();
    int max = seq->chromosomes[ui->tracks->range.chromosome].length() - visible_range + 1 ;
    ui->positionSlider->setRange(1,max);
    on_positionSlider_valueChanged(ui->positionSlider->value());
//    qDebug() << "MainWindow::on_zoomSlider_valueChanged end";
}

void MainWindow::on_browserSizeSlider_valueChanged(int value) {
//    qDebug() << "MainWindow::on_browserSlider_valueChanged";
    TrackVariation *var = (TrackVariation*) ui->tracks->getTrack("variation");
    if (var) {
        ui->lineSize->setText(QString::number(value));
        var->sample_height = value;
        ui->tracks->recalculateLayout();
        ui->tracks->checkAndPaint();
    }
}

void MainWindow::on_nonCodingCheckBox_clicked() {
    ui->tracks->changeFilter();
    ui->tracks->update();
}

void MainWindow::on_synonymousCheckBox_clicked() {
    ui->tracks->changeFilter();
    ui->tracks->update();
}

void MainWindow::on_nonSynonymousCheckBox_clicked() {
    ui->tracks->changeFilter();
    ui->tracks->update();
}

void MainWindow::on_indelCheckBox_clicked() {
    ui->tracks->changeFilter();
    ui->tracks->update();
}

void MainWindow::on_ungroupedCheck_clicked() {
    ui->tracks->recalculateLayout();
    ui->tracks->changeGroups();
    ui->tracks->update();
}

void MainWindow::track_checkbox_clicked(){
    ui->tracks->trackenabledisable();
}

void MainWindow::viewGroupManager() {
    GroupManager gm(this);
    gm.setModal(true);
    gm.exec();
}

void MainWindow::addGroup(QString group) {
    QString name = group;
    name.replace(" ", "");

    QFont font;
    font.setPointSize(9);

    QCheckBox *newCheck = new QCheckBox(ui->groupsWidget);

    newCheck->setObjectName("group" + name + "Check");
    newCheck->setGeometry(0, 21 * groups.count(), ui->groupsWidget->width(), 21);

    newCheck->setFont(font);
    newCheck->setText(group);
    newCheck->setChecked(true);
    newCheck->show();

    groups[group] = newCheck;
    connect(newCheck, SIGNAL(clicked()), this, SLOT(on_ungroupedCheck_clicked()));
    ui->groupsWidget->setMinimumHeight(21 * groups.count());

    int groupID = ui->tracks->getVariation()->groups.size()-1;
    QColor color = this->assignColor(groupID);
    ui->tracks->getVariation()->groupColor.insert(group,color);
    updateGroupWidget();
}

void MainWindow::on_lineQuality_editingFinished() {
    bool ok;
    int value = ui->lineQuality->text().toInt(&ok);
    if (ok) {
        ui->qualitySlider->setValue(value);
    } else {
        ui->lineQuality->setText(QString::number(ui->qualitySlider->value()));
    }
}

void MainWindow::on_lineDepth_editingFinished() {
    bool ok;
    int value = ui->lineDepth->text().toInt(&ok);
    if (ok) {
        ui->readDepthSlider->setValue(value);
    } else {
        ui->lineDepth->setText(QString::number(ui->readDepthSlider->value()));
    }
}

void MainWindow::on_lineSize_editingFinished() {
    bool ok;
    int value = ui->lineSize->text().toInt(&ok);
    if (ok) {
        ui->browserSizeSlider->setValue(value);
    } else {
        ui->lineSize->setText(QString::number(ui->browserSizeSlider->value()));
    }
}

void MainWindow::viewAboutPage() {
    QMessageBox msgBox;
    msgBox.setWindowTitle("About SVAMP");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(QString("<font size='+2'>SVAMP v2.10</font><br/><br/>") +
                   "Developed by Raeece Naeem and Lailatul Hidayah<br/>" +
                   "Licence: GPL<br/><br/>" +
                   "<br/>" +
                   "SVAMP is a tool to visualize variants (SNPs and indels) and perform realtime analysis on selected regions of a genome "+
                   "on specific samples. SVAMP has some interesting features like phylogeography, allele frequency map and Principal Coordinate "+
                   "Analysis(PCoA). SVAMP takes VCF files (Variant Call Format) as input. Metainformation on samples are simple tab delimited text "+
                   "files. In addition SVAMP also reads annotation files in GFF format and reference sequences in FASTA format.<br/>"+
                   "<br/>" +
                   "<a href='http://www.cbrc.kaust.edu.sa/svamp'>SVAMP Website</a><br/>" +
                   "<br/>" +
                   "<a href='http://www.cbrc.kaust.edu.sa/svamp/downloads/svamp_manual.pdf'><font size='+1'>Help</font></a>");
    msgBox.exec();

}

void MainWindow::editColour() {
    Colors c(this);
    c.setModal(true);
    c.exec();
}

void MainWindow::saveAction() {
    Save s(this);
    s.setModal(true);
    s.exec();
}

void MainWindow::on_positionLeftButton_clicked() {
    long from, to;
    from = ui->lineFrom->text().toLong();
    to = ui->lineTo->text().toLong();
    ui->positionSlider->setValue(2*from - to);
}

void MainWindow::on_positionRightButton_clicked() {
    ui->positionSlider->setValue(ui->lineTo->text().toLong() + 1);
}

void MainWindow::viewLog() {
    logBrowser->show();;
}

void MainWindow::viewUncovered()
{
    BaseCoverageDialog bcd(this);
    bcd.setModal(true);
    bcd.exec();
}

void MainWindow::updateVariationTrack()
{
    coveragePainting=true;
    ui->tracks->checkAndPaint();
}

QColor MainWindow::assignColor(int groupID)
{
    QColor color;
    if (groupID%12==0)
        color= Qt::darkYellow;
    else if (groupID%12==1)
        color= Qt::green;
    else if (groupID%12==2)
        color= Qt::black;
    else if (groupID%12==3)
        color= Qt::blue;
    else if (groupID%12==4)
        color= Qt::red;
    else if (groupID%12==5)
        color= Qt::cyan;
    else if (groupID%12==6)
        color= Qt::magenta;
    else if (groupID%12==7)
        color= Qt::darkRed;
    else if (groupID%12==8)
        color= Qt::darkGreen;
    else if (groupID%12==9)
        color= Qt::darkBlue;
    else if (groupID%12==10)
        color= Qt::darkCyan;
    else if (groupID%12==11)
        color= Qt::darkMagenta;
    return color;
}

void MainWindow::runPcoa(bool allSamples, QList<VariationPosition*>  vpList, bool indels)
{
    qDebug()<< QDateTime::currentDateTime().toString("yyyy-mm-dd hh:mm:ss")<< "MainWindow::runPcoa start";
    Variation * var= ui->tracks->getVariation();
    PCOA calc;

    if(allSamples)
    {
        //ui->listWidget->selectAll();
        this->selectedItems.clear();
        foreach(QString sample, ui->tracks->getVariation()->samples)
            this->selectedItems.append(sample);
    }

    if (calc.constructDissimilarityMatrix(var, this->selectedItems,vpList, indels )!=-1)
    {
        calc.runPcoaAlgorithm(this->selectedItems.count());
        PCOADialog *pcad = new PCOADialog(this);

        pcad->initData(calc.finalDataTransposed, !allSamples, this->selectedItems);
        pcad->getUI()->label_nsnp->setText(QString::number(calc.nVars)+ " variants");
        qDebug()<< QDateTime::currentDateTime().toString("yyyy-mm-dd hh:mm:ss")<< "MainWindow::runPcoa end";
        pcad->showMaximized();
    }
}

void MainWindow::viewMap()
{
    MapDialog *mapdialog = new MapDialog(this);

}

void MainWindow::viewAlleleMap()
{
    if(this->clickedPosition>0)
    {
        MapAlleleDialog * mapd = new MapAlleleDialog(this);
        if(mapd->databaseOK){
          mapd->show();
        }
        else{
            QMessageBox nodb(QMessageBox::Critical, "No db Error ", "Please load sample information .db file ", QMessageBox::Ok);
            nodb.exec();
        }

    }else
    {
        QMessageBox warning(QMessageBox::Critical, "Map Error ", "No SNP position selected", QMessageBox::Ok);
        warning.exec();
    }
}

void MainWindow::on_lookSeqPushButton_clicked()
{
    int start=ui->tracks->range.from;
    if (this->clickedPosition>0)
    {
        start = clickedPosition;
    }

    QString chromName = ui->chrBox->currentText();
    QStringList selectedSampleNamesList;
    if(ui->listWidget->selectedItems().isEmpty())
    {
        QMessageBox warning(QMessageBox::Critical, "Error ", "No sample selected", QMessageBox::Ok);
        warning.exec();
    }else
    {
        for (int k=0;k<ui->listWidget->selectedItems().size();k++)
            selectedSampleNamesList.append(ui->listWidget->selectedItems().at(k)->text());

        if(locatePosition(start,chromName,selectedSampleNamesList)<0)
        {
            QMessageBox warning(QMessageBox::Critical, "Error ", "Please specify the jBrowse/LookSeq path", QMessageBox::Ok);
            warning.exec();
        }
    }
}

void MainWindow::createMenu()
{
    load = new QAction("&Load", this);
    save = new QAction("&Save Main View", this);
    log = new QAction("&Log", this);

    jbrowseSetting = new QAction("&BAM Viewer URL", this);

    tree = new QAction("&Tree", this);
    pcoaall = new QAction("&All", this);

    pcoaselected = new QAction("&Selected", this);
    viewuncovered = new QAction("View Uncovered Region", this);
    mapaction = new QAction("&Map with Year Slider", this);
    allelemapact = new QAction("&Allele Map",this);

    about = new QAction("&About", this);

    grouping = new QAction("&Grouping", this);
    colourcode = new QAction("&Colour Coding", this);

    file = menuBar()->addMenu("&File");
    file->addAction(load);
    file->addAction(save);
    file->addAction(log);
    file->addSeparator();
    file->addAction(jbrowseSetting);
    file->addAction(about);

    tools = menuBar()->addMenu("&Tools");
    tools->addAction(tree);

    pcoa = tools->addMenu("&PCOA");
        pcoa->addAction(pcoaall);
        pcoa->addAction(pcoaselected);

    tools->addAction(mapaction);
    tools->addAction(allelemapact);
    tools->addAction(viewuncovered);

    edit = menuBar()->addMenu("&Edit");
    edit->addAction(grouping);
    edit->addAction(colourcode);

    blues = new QAction("&Blue", this);
    greens = new QAction("&Green", this);
    limes = new QAction("&Limes", this);
    style= edit->addMenu("&Theme");
        style->addAction(blues);
        style->addAction(greens);
        style->addAction(limes);

    connect(load, SIGNAL(triggered()), this, SLOT(loadFilesAction()));
    connect(save, SIGNAL(triggered()), this, SLOT(saveAction()));
    connect(jbrowseSetting, SIGNAL(triggered()),this, SLOT(launchJBrowseSetting()));

    connect(tree, SIGNAL(triggered()), this, SLOT(launchTreeComputation()));
    connect(pcoaall, SIGNAL(triggered()), this, SLOT(launchSNPForPCOA()));
    connect(pcoaselected, SIGNAL(triggered()), this, SLOT(launchSNPForPCOASelected()));
    connect(viewuncovered, SIGNAL(triggered()), this, SLOT(viewUncovered()));
    connect(mapaction, SIGNAL(triggered()), this, SLOT(viewMap()));
    connect(allelemapact,SIGNAL(triggered()),this,SLOT(viewAlleleMap()));
 //   connect(snpselect,SIGNAL(triggered()),this, SLOT(launchSNPSelection()));

    connect(about, SIGNAL(triggered()), this, SLOT(viewAboutPage()));
    connect(log, SIGNAL(triggered()), this, SLOT(viewLog()));

    connect(grouping, SIGNAL(triggered()), this, SLOT(viewGroupManager()));
    connect(colourcode, SIGNAL(triggered()), this, SLOT(editColour()));

    connect(blues, SIGNAL(triggered()),this, SLOT(switchToBlue()));
    connect(greens, SIGNAL(triggered()),this, SLOT(switchToGreen()));
    connect(limes, SIGNAL(triggered()),this, SLOT(switchToLime()));

}

void MainWindow::launchTreeComputation()
{
     //qDebug() << "MainWindow::launchTreeComputation is called";
    tree->setEnabled(false);
    TreeOptionsDialog * tod = new TreeOptionsDialog(this);
    connect(tod, SIGNAL(activateTreeMenu()), this, SLOT(enableTreeMenuOnly()));

    tod->setModal(false);
    tod->show();
}

void MainWindow::showDetails()
{
    QString html="";
    QString chrom = ui->chrBox->currentText();
    html.append("<html><table>"\
                "<tr><td>Sequence: </td><td>" +QString::number(ui->tracks->getSequence()->chromosomes.size()) + "</td></tr>"\
                "<tr><td>Annotation:</td><td>" + QString::number(ui->tracks->getAnnotation()->getTotalRecord()) + " records</td></tr>"\
                "<tr><td>Variants:</td><td>"+QString::number(ui->tracks->getVariation()->getTotalVariants())+" records </td></tr>"\
                "<tr></tr>"\
                "<tr><td></td><td>Current Chrom</td></tr>"\
                "<tr><td>SNP:</td><td>"+ QString::number(ui->tracks->getVariation()->number[chrom].nSNP) +"</td></tr>"\
                 "<tr><td>Indel:</td><td>"+ QString::number(ui->tracks->getVariation()->number[chrom].nIndel) +"</td></tr>"\
                 "<tr><td>Missing:</td><td>"+ QString::number(ui->tracks->getVariation()->number[chrom].nMissing) +"</td></tr>"\
                  "<tr><td>Ref:</td><td>"+ QString::number(ui->tracks->getVariation()->number[chrom].nRef) +"</td></tr>"\
                "</table></html>");
    ui->textBrowser->setText(html);
}

void MainWindow::on_commonCheckBox_clicked()
{
    ui->tracks->changeFilter();
    ui->tracks->update();
}

void MainWindow::disableDataMenu()
{
    //tree->setEnabled(false);
    pcoaall->setEnabled(false);
    pcoaselected->setEnabled(false);

    ui->rsdButton->setEnabled(false);
    ui->findResults->setEnabled(false);

    viewuncovered->setEnabled(false);

    grouping->setEnabled(false);
}

void MainWindow::enableDataMenu()
{
    tree->setEnabled(true);
    pcoaall->setEnabled(true);
    pcoaselected->setEnabled(true);

    ui->rsdButton->setEnabled(true);
    ui->findResults->setEnabled(true);

    viewuncovered->setEnabled(true);
    grouping->setEnabled(true);
}

void MainWindow::enableTreeMenuOnly()
{
    tree->setEnabled(true);
}

void MainWindow::applyInitialGrouping()
{
    DBQuery dbq;
    dbq.connect();

    QString country;
    QStringList sampleList;
    int index;

    if (dbq.isConnected())
    {
        QMap<QString, QStringList> sampleOriPair= dbq.querySampleOrigin();
        QStringList keys = sampleOriPair.keys();
        for(int y=0;y<sampleOriPair.size();y++)
        {
            country = keys.at(y);
            sampleList = sampleOriPair[country];
            foreach(QString sample, sampleList)
            {
                index = ui->tracks->getVariation()->samples.indexOf(sample);//return index of sample
                if(ui->tracks->getVariation()->groups.contains(country))
                {
                    if (-1 != index && sample!="") {
                        ui->tracks->getVariation()->groups["Ungrouped"].removeOne(index);
                        ui->tracks->getVariation()->groups[country].append(index);
                    }
                }else
                {
                    //add new group :
                    ui->tracks->getVariation()->groups[country];
                    this->addGroup(country);
                    if (-1 != index && sample!="") {
                        ui->tracks->getVariation()->groups["Ungrouped"].removeOne(index);
                        ui->tracks->getVariation()->groups[country].append(index);
                    }
                }
            }
        }
    }

    dbq.closeDB();

    //QDir::setCurrent(QCoreApplication::applicationDirPath());
    QFile file("groupscolors.pv");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream fin(&file);
        QString line = fin.readLine();
        while(!line.isNull())
        {
            QStringList gcolorpair = line.split("\t");
            if(ui->tracks->getVariation()->groupColor.contains(gcolorpair.at(0)))
                ui->tracks->getVariation()->groupColor[gcolorpair.at(0)]=QColor(gcolorpair.at(1));
            line=fin.readLine();
        }
        file.close();
    }

    updateGroupWidget();
}

void MainWindow::launchJBrowseSetting()
{
    JBrowseSettingDialog lsd;
    lsd.exec();
}

void MainWindow::launchSNPForPCOA()
{
    SNPSelectionDialog * ssd = new SNPSelectionDialog(this);
    ssd->pcoa = true;
    ssd->areAllSamples = true;
    ssd->show();
}

void MainWindow::launchSNPForPCOASelected()
{
    if(ui->listWidget->selectedItems().isEmpty())
    {
        QMessageBox warning(QMessageBox::Critical, "Selection Error", "No sample selected for PCoA computation", QMessageBox::Ok);
        warning.exec();
        return;
    }

    this->selectedItems.clear();
    foreach(QListWidgetItem * item, ui->listWidget->selectedItems())
        this->selectedItems.append(item->text());

    SNPSelectionDialog * ssd = new SNPSelectionDialog(this);
    ssd->pcoa = true;
    ssd->areAllSamples = false;
    ssd->show();
}

void MainWindow::clearMenuPointers()
{
    delete file;
    delete tools;
    delete edit;

    delete load;
    delete save;
    delete log;
    delete tree;
    delete pcoaall;
    delete pcoaselected;
    delete viewuncovered;
    delete mapaction;
    delete about;
    delete grouping;
    delete colourcode;
    delete allelemapact;
}

void MainWindow::switchToGreen()
{
    QFile file;
    file.setFileName(":/qss/svamp_mac_green.qss");

#ifdef WIN32
     file.setFileName(":/qss/svamp_win32_green.qss");
#endif

    if(!file.open(QFile::ReadOnly))
        qDebug() << file.error();

    qApp->setStyleSheet(file.readAll());

    //return colors of qcheckbox
    if (this->groups.size()>1)
    {
        foreach(QString groupnm, this->groups.keys())
        {
            QColor color = ui->tracks->getVariation()->groupColor[groupnm];
            int red = color.red();
            int green = color.green();
            int blue = color.blue();
            this->groups[groupnm]->setStyleSheet("QCheckBox { color:rgb("+QString::number(red)+"," + QString::number(green)+ "," + QString::number(blue)+") }");
        }
    }
}

void MainWindow::updateGroupWidget()
{
    /*updating the color setting of all checkbox in this groupWidget*/
    QObjectList checkBoxList = ui->groupsWidget->children();
    QPalette plt;
    foreach(QObject * cb, checkBoxList)
    {
        if(ui->tracks->getVariation()->groups.contains(((QCheckBox*)cb)->text()))//check if the group in the current checkbox is still exist
        {
            plt.setColor(QPalette::WindowText, ui->tracks->getVariation()->groupColor[((QCheckBox*)cb)->text()]);
            ((QCheckBox*)cb)->setPalette(plt);
        }else
        {
            ((QCheckBox*)cb)->hide();
            ((QCheckBox*)cb)->deleteLater();
        }
    }
}

void MainWindow::switchToBlue()
{
    QFile file;
    file.setFileName(":/qss/svamp_mac.qss");

#ifdef WIN32
     file.setFileName(":/qss/svamp_win32.qss");
#endif

    if(!file.open(QFile::ReadOnly))
        qDebug() << file.error();

    qApp->setStyleSheet(file.readAll());

    //return colors of qcheckbox
    if (this->groups.size()>1)
    {
        foreach(QString groupnm, this->groups.keys())
        {
            QColor color = ui->tracks->getVariation()->groupColor[groupnm];
            int red = color.red();
            int green = color.green();
            int blue = color.blue();
            this->groups[groupnm]->setStyleSheet("QCheckBox { color:rgb("+QString::number(red)+"," + QString::number(green)+ "," + QString::number(blue)+") }");
        }
    }
}

void MainWindow::switchToLime()
{
    QFile file;
    file.setFileName(":/qss/svamp_mac_lime.qss");

#ifdef WIN32
     file.setFileName(":/qss/svamp_win32_lime.qss");
#endif

    if(!file.open(QFile::ReadOnly))
        qDebug() << file.error();

    qApp->setStyleSheet(file.readAll());

    //return colors of qcheckbox
    if (this->groups.size()>1)
    {
        foreach(QString groupnm, this->groups.keys())
        {
            QColor color = ui->tracks->getVariation()->groupColor[groupnm];
            int red = color.red();
            int green = color.green();
            int blue = color.blue();
            this->groups[groupnm]->setStyleSheet("QCheckBox { color:rgb("+QString::number(red)+"," + QString::number(green)+ "," + QString::number(blue)+") }");
        }
    }
}

int MainWindow::locatePosition(int start, QString chromName, QStringList isolateID)
{
    //QDir::setCurrent(QCoreApplication::applicationDirPath());
    QFile temp("svamp.temp");
    QString jbrowsePath, lookseqPath;
    if(temp.open(QIODevice::ReadOnly| QIODevice::Text))
    {
        QTextStream fin(&temp);
        fin.readLine();//the first line is db path
        jbrowsePath = fin.readLine();
        lookseqPath = fin.readLine();
        temp.close();
    }else
        qDebug() << "Svamp.temp couldnt be found";

    if(ui->sampleBox->currentIndex()==1)//jbrowse is selected
    {
        if(jbrowsePath.isEmpty())
        {
            return -1;
        }
        else
        {
            jbrowsePath.append("&loc=");
            jbrowsePath.append(chromName);
            jbrowsePath.append(":");

            int end=start+72;
            QString startPosition= QString::number(start);
            jbrowsePath.append(startPosition);
            jbrowsePath.append("..");
            QString endPosition= QString::number(end);

            jbrowsePath.append(endPosition);
            jbrowsePath.append("&tracks=DNA,");
            QString label;
            int nSelectedSample=isolateID.size();
            for(int i=0;i<nSelectedSample;i++)
            {
                QString id=isolateID.at(i);
                label= id+" SNPCoverage,"+id+" Alignment";

                if(i<nSelectedSample-1)label+=",";

                jbrowsePath.append(label);
            }
            jbrowsePath.append("&highlight=");
            qDebug() << "Loaded jBrowse Path : " << jbrowsePath;
            QDesktopServices::openUrl(jbrowsePath);
            return 0;
        }// end of else for if jbrowse path has been specified
    }else
    {
        if(lookseqPath.isEmpty())
            return -2;
        else
        {
            lookseqPath.append("/index.html?chr=");
            lookseqPath.append(chromName);

            int end=start+160;//confirm this formula
            QString startPosition= QString::number(start);
            lookseqPath.append("&from=");
            lookseqPath.append(startPosition);
            lookseqPath.append("&to=");
            QString endPosition= QString::number(end);

            lookseqPath.append(endPosition);
            lookseqPath.append("&mode=indel&samples=");
            lookseqPath.append(isolateID.at(0));
            lookseqPath.append("|smalt&options=perfect|1,snps|1,single|0,inversions|1,pairlinks|1,orientation|0,faceaway|0,basequal|0&maxdist=500");
            //ui->webView->load(QUrl("http://localhost/cgi-bin/index.pl?show=ENA_AL123456_AL123456.2:255-415,paired_pileup&lane=191_smaltsorted.bam&width=1024&win=160&display=|perfect|single|inversions|pairlinks|potsnps|uniqueness|gc|coverage|annotation|gc|coverage|&maxdist=500"));
            QDesktopServices::openUrl(lookseqPath);
            //ui->webView->load(path);
            return 0;
        }
    }

}

QList<long> MainWindow::getVarPosBetween(long x1, long x2)
{
    QList<long> posList;
    if(x2<x1) return posList;

    Variation * var = ui->tracks->getVariation();
    QString chromosome = ui->chrBox->currentText();
    int nSnps = var->data[chromosome].size();
    QList<VariationPosition*> vpList = var->data[chromosome];

    for (int i=0; i< nSnps; i++)//iterating through the SNPs
    {
        VariationPosition *v = vpList[i];

        if (v->position <x1) continue; // Assuming VCF file was sorted by position...

        if (v->position >= x1 && v->position <= x2)
        {
           posList.append(v->position);
           //qDebug() << "getVarPosBetween::appending " << v->position;
        }

        if(v->position>x2) break;
    }
    return posList;
}

