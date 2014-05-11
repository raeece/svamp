#include "treeoptionsdialog.h"
#include "ui_treeoptionsdialog.h"
#include "treeviewdialog.h"
#include "pcoa.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QCloseEvent>
#include <QProgressDialog>
#include <QApplication>
#include <QThread>
#include "nexusutils.h"
#include "lexer.h"
#include "parser.h"


extern "C" int yyparse (void);
FILE *outnewickfile;
FILE *parserrorfile;


TreeOptionsDialog::TreeOptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TreeOptionsDialog)
{
    ui->setupUi(this);
    ui->comboBox_method->setCurrentIndex(1);
    ui->frame_advancedSet->setEnabled(false);

    showMore=false;
    progressdialog = new QProgressDialog(this);

    if (MainWindow::instance->getUI()->chrBox->count()>0)//if data is loaded
    {
        QStringList sampleList = MainWindow::instance->getUI()->tracks->getVariation()->samples;
        foreach(QString name, sampleList)
            ui->comboBox_sampleList->addItem(name);
        ui->comboBox_sampleList->setCurrentIndex(0);
    }
}

TreeOptionsDialog::~TreeOptionsDialog()
{
    delete ui;
}

void TreeOptionsDialog::closeEvent(QCloseEvent *)
{
    emit activateTreeMenu();
    accept();
}

void TreeOptionsDialog::saveAlignment(QList<VariationPosition *> vpList, bool nonCoding,bool selectedsamples){

    bool indelsExcluded=false;

    qDebug()<<"inside save alignment";

    TrackContainer * tr = MainWindow::instance->getUI()->tracks;
    Variation * var= tr->getVariation();
    if(selectedsamples){

    MainWindow::instance->selectedItems.clear();
    foreach(QListWidgetItem * item, MainWindow::instance->getUI()->listWidget->selectedItems()){
        MainWindow::instance->selectedItems.append(item->text());
        }
    if(MainWindow::instance->selectedItems.isEmpty())
    {
        QMessageBox warning(QMessageBox::Critical, "Selection Error", "No samples selected", QMessageBox::Ok);
        warning.exec();
        return;
    }
    }
    else{
        for(int i = 0; i <  MainWindow::instance->getUI()->listWidget->count(); ++i){
        QListWidgetItem * item= MainWindow::instance->getUI()->listWidget->item(i);
        MainWindow::instance->selectedItems.append(item->text());
        }
    }
    QStringList selectedList=MainWindow::instance->selectedItems;
    qDebug()<<"selectedItems"<<selectedList;
    //QProgressDialog progress("Saving alignment files...", "Abort", 1, 5, MainWindow::instance);
    //progress.setWindowModality(Qt::WindowModal);
    progressdialog->setLabelText("Saving alignment files...");
    progressdialog->setRange(1,5);
    progressdialog->setModal(false);
    progressdialog->setAutoClose(true);
    progressdialog->setValue(1);
    progressdialog->show();

    // number of samples
    int nPoints = var->samples.size();

    //calculate number of considered SNPs
    int nVars= vpList.size();

    //store index of selected samples
    QList<int> selectedIndex;
            for (int s=0;s<nPoints;s++)
            {
                if(PCOA::isSelectedSample(var->samples[s],selectedList))
                {
                    selectedIndex.append(s);
                }
            }

     int nSelected = selectedIndex.count();
     qDebug() <<"nVars="<<nVars<<"nPoints"<<nPoints<<"nSelected"<<nSelected<<"\n";


    QList < QList <QString> > align_matrix;

    for (int i=0; i< nVars; i++)//iterating through the SNPs
     {
        QList <QString> tobealigned;
        QString ref;
        bool msaflag=false;
        VariationPosition *vp;
        VariationData gt;
        //fout<<"pos="<<i<<"\t";
           for (int z=0;z<nSelected;z++) //iterating through samples
           {
             int x=selectedIndex.at(z);
             //fout<<var->samples[x]<<"\t";

             vp = vpList.at(i);
             gt = vp->data[x];

             if(!indelsExcluded){
                 //  set the strongest haplotype to the 'most' alternate.
                 int genotype = gt.haplotype1 > gt.haplotype2 ? gt.haplotype1 : gt.haplotype2;
                 if(gt.insertion){
                     //fout<<"+"<<vp->genotypes[genotype]<<"+";
                     msaflag=true;
                     tobealigned.append(vp->genotypes[genotype]);
                 }
                 if(gt.deletion){
                     //fout<<"-"<<vp->genotypes[genotype]<<"-";
                     msaflag=true;
                     tobealigned.append(vp->genotypes[genotype]);
                 }

             }
             if(!(gt.insertion || gt.deletion)){
             //fout<<gt.haplotypeStrong;
             tobealigned.append(gt.haplotypeStrong);
             }
             ref=vp->genotypes[0];
          }
           if(msaflag){

               QList <QString> aligned_genotypes=leftMSA(ref,tobealigned);
               align_matrix.append(aligned_genotypes);
           }
           else{
               align_matrix.append(tobealigned);
           }
           msaflag=false;
           //fout<<"\n";
      }

    progressdialog->setValue(1);
    QDir directory(ui->lineEdit_alignFile->text());
    QString path = directory.filePath("align.nex");
    QFile nexusfile(path);
    nexusfile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream fout(&nexusfile);


    // save as Nexus sequential format
    fout<<"#NEXUS\n";
    fout<<"BEGIN TAXA;\n";
    fout<<"dimensions ntax="<<nSelected<<";\n";
    fout<<"TAXLABELS\n";
    for (int z=0;z<nSelected;z++) //iterating through samples
    {
        int x=selectedIndex.at(z);
        fout<<var->samples[x]<<"\n";
    }
    fout<<";\n";
    fout<<"END;\n\n";

    int ncharslength=0;
    for (int i=0;i<align_matrix.length();i++){
        QString str=align_matrix.at(i).at(0);
        ncharslength+=str.length();
    }
    // print the characters
    fout<<"BEGIN CHARACTERS;\n";
    fout<<"dimensions nchar="<<ncharslength<<";\n";
    fout<<"format datatype=dna gap=-;\n";
    fout<<"Matrix\n";
    for (int z=0;z<nSelected;z++) //iterating through samples
    {
        int x=selectedIndex.at(z);
        fout<<var->samples[x]<<" ";
        for (int i=0;i<align_matrix.length();i++){
            QString str=align_matrix.at(i).at(z);
            fout<<str;
        }
        fout<<"\n";
    }
    fout<<";\n";
    fout<<"end;\n";
     nexusfile.close();

     QHash <QString,QString> phylip_sample_names;
     QHash <QString,QString> t_sample_names;
     // save in fasta format
      int longest_name_length=0;
      path = directory.filePath("align.fasta");
      QFile fastafile(path);
      fastafile.open(QIODevice::WriteOnly | QIODevice::Text);
      fout.setDevice(&fastafile);
      for (int z=0;z<nSelected;z++) //iterating through samples
      {
          int x=selectedIndex.at(z);
          fout<<">"<<var->samples[x]<<"\n";
          // length of the longest taxon name is needed for padding in relaxed phylip format
          if(var->samples[x].length()>longest_name_length){
              longest_name_length=var->samples[x].length();
          }
          t_sample_names.insertMulti(var->samples[x].leftJustified(10,' ',true),var->samples[x]);
          phylip_sample_names.insert(var->samples[x],var->samples[x].leftJustified(10,' ',true));
          for (int i=0;i<align_matrix.length();i++){
              QString str=align_matrix.at(i).at(z);
              fout<<str;
          }
          fout<<"\n";
      }
      fastafile.close();
      progressdialog->setValue(2);
    // for phylip format sample names should be restricted to 10 chars padded with space
      int suffix=1;
      QString suffixstr=QString::number(suffix);
      QSet <QString> uniqkeys=t_sample_names.keys().toSet();
      QSet <QString> newuniqkeys=uniqkeys;
      QSetIterator <QString> i(uniqkeys);
      while (i.hasNext()) {
          QString k=i.next();
          QList <QString> uniqvals=t_sample_names.values(k);
          if(uniqvals.length()>1){
              for (int u=0;u<uniqvals.length();u++){
                   int prefix_length=10-suffixstr.length();
                   QString newkey=k.left(prefix_length).append(suffixstr);
                   // keep generating new key that doesnt exist already
                   while(newuniqkeys.contains(newkey)){
                    suffix++;
                     //qDebug()<<"suffix in while loop"<<suffix;
                    suffixstr=QString::number(suffix);
                    int prefix_length=10-suffixstr.length();
                    newkey=k.left(prefix_length).append(suffixstr);
                   }
                   phylip_sample_names.insert(uniqvals[u],newkey);
                   newuniqkeys.insert(newkey);
                   //qDebug()<<k<<" len="<<uniqvals.length()<<" "<<uniqvals[u]<<":"<<newkey;
                   suffix++;
                   suffixstr=QString::number(suffix);
                   //qDebug()<<"suffix "<<suffix;
              }
          }
      }
    // save phylip dictionary
       path = directory.filePath("align.phy.map");
       QFile phymap(path);
       phymap.open(QIODevice::WriteOnly | QIODevice::Text);
       fout.setDevice(&phymap);
           QHashIterator<QString, QString> j(phylip_sample_names);
           while (j.hasNext()) {
               j.next();
               fout << j.key() << ": " << j.value() << endl;
           }
        phymap.close();

    progressdialog->setValue(3);
    // save in phylip format
     path = directory.filePath("align.phy");
     QFile phylipfile(path);
     phylipfile.open(QIODevice::WriteOnly | QIODevice::Text);
     fout.setDevice(&phylipfile);
     fout<<" "<<nSelected<<" "<<ncharslength<<" s\n";
     for (int z=0;z<nSelected;z++) //iterating through samples
     {
         int x=selectedIndex.at(z);
         //fout<<var->samples[x]<<" ";
         fout<<phylip_sample_names.value(var->samples[x])<<" ";
         for (int i=0;i<align_matrix.length();i++){
             QString str=align_matrix.at(i).at(z);
             fout<<str;
         }
         fout<<"\n";
     }
     phylipfile.close();

     progressdialog->setValue(4);
     // save in relaxed phylip format for programs like raxml
      path = directory.filePath("align.phyr");
      QFile rphylipfile(path);
      rphylipfile.open(QIODevice::WriteOnly | QIODevice::Text);
      fout.setDevice(&rphylipfile);
      fout<<nSelected<<" "<<ncharslength<<"\n";
      for (int z=0;z<nSelected;z++) //iterating through samples
      {
          int x=selectedIndex.at(z);
          //fout<<var->samples[x]<<" ";
          fout<<var->samples[x].leftJustified(longest_name_length,' ',true)<<" ";
          for (int i=0;i<align_matrix.length();i++){
              QString str=align_matrix.at(i).at(z);
              fout<<str;
          }
          fout<<"\n";
      }
      rphylipfile.close();
    progressdialog->setValue(5);
    QMessageBox notification(QMessageBox::Information,"Alignment saved", "Alignments saved in location  "+ui->lineEdit_alignFile->text(), QMessageBox::Ok);
     notification.exec();
     return;



}

void TreeOptionsDialog::computeTree(QList<VariationPosition *> vpList, bool nonCoding,bool selectedsamples)
{
    qDebug()<< QDateTime::currentDateTime().toString("yyyy-mm-dd hh:mm:ss")<< "TreeOptionsDialog::computeTree start";

    if(selectedsamples){
    MainWindow::instance->selectedItems.clear();
    foreach(QListWidgetItem * item, MainWindow::instance->getUI()->listWidget->selectedItems()){
        MainWindow::instance->selectedItems.append(item->text());
        }
    if(MainWindow::instance->selectedItems.isEmpty())
    {
        QMessageBox warning(QMessageBox::Critical, "Selection Error", "No samples selected", QMessageBox::Ok);
        warning.exec();
        return;
    }
    }
    else{
        for(int i = 0; i <  MainWindow::instance->getUI()->listWidget->count(); ++i){
        QListWidgetItem * item= MainWindow::instance->getUI()->listWidget->item(i);
        MainWindow::instance->selectedItems.append(item->text());
        }
    }
    QStringList selectedList=MainWindow::instance->selectedItems;
    qDebug()<<"selectedItems"<<selectedList;


    //generate a file of dissimilarity matrix with format acceptable by Phylip
    if(generatePhylipMatrix(vpList, nonCoding,selectedList)<0)
    {
        qDebug() << "generatePhylipMatrix(vpList) returns less than 0";
        ui->pushButton_ok->setEnabled(true);
        return;
    }else
    {
        //read dissimilarity matrix file
        QFile inputfile("matrixInPhylipFormat.txt");
        if (! inputfile.exists()) {
            QMessageBox msgBox;
            msgBox.setText("No input file: " + inputfile.fileName());
            msgBox.exec();
            return;
        }
        inputfile.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream in(&inputfile);
        QString line = in.readLine();
        int nSample = line.toInt();
        //int num = ui->lineEdit_spNum->text().toInt();
        int num=1;
        if (MainWindow::instance->getUI()->chrBox->count()>0)// if data is loaded
            if(selectedsamples){
            num = selectedList.indexOf(ui->comboBox_sampleList->currentText())+1;
            }
            else{
            num = MainWindow::instance->getUI()->tracks->getVariation()->samples.indexOf(ui->comboBox_sampleList->currentText())+1;
            }
        if (nSample>0 && num>0 && num<=nSample)
        {
            worker = new Worker;
            this->donesamples=0;

            progressdialog->setLabelText(tr("Generating Tree..."));
            progressdialog->setRange(0,nSample);
            progressdialog->setModal(false);
            progressdialog->setAutoClose(true);
            progressdialog->setValue(this->donesamples);
            progressdialog->show();

            connect(worker, SIGNAL(started(TreeCalculation*)), this, SLOT(GetOptions(TreeCalculation*)));
            connect(this, SIGNAL(optionsSaved(TreeCalculation*)), worker, SLOT(startComputing(TreeCalculation*)));
            connect(worker, SIGNAL(incremented()), this, SLOT(incrementDone()));
            connect(worker, SIGNAL(finishSignal()), progressdialog, SLOT(close()));

            //connect(worker, SIGNAL(finishSignal()), thread, SLOT(quit()));
            //connect(worker, SIGNAL(finishSignal()), worker, SLOT(deleteLater()));
            //connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

            connect(worker, SIGNAL(errorOccur(QString)), progressdialog, SLOT(close()));
            connect(worker, SIGNAL(errorOccur(QString)), this, SLOT(fireErrorMessage(QString)));

            //worker->moveToThread(thread);
            //thread->start();
            //QMetaObject::invokeMethod(worker, "doWork", Qt::QueuedConnection);

            worker->doWork();

//            progressdialog.setValue(this->donesamples);
//            progressdialog.show();
            ui->pushButton_ok->setEnabled(false);

            //while(!worker->successfullwork)
            //{

//                progressdialog.setValue(this->donesamples);

//                qApp->processEvents();
//                if (progressdialog.wasCanceled())
//                {
//                    //emit signal that it was cancelled, should be catched by treecalculation
//                    worker->canceltree();
//                    ui->pushButton_ok->setEnabled(true);
//                    break;
//                }
//                if(worker->successfullwork)
//                {
//                    //emit finished signal
//                    progressdialog.hide();
//                    break;
//                }

//            }//end of while thread is running

            // It is now safe to display the tree
            hide();
            qDebug()<< QDateTime::currentDateTime().toString("yyyy-mm-dd hh:mm:ss")<< "TreeOptionsDialog::computeTree end";
            viewTree();

        }else//end of conditional nSample>0
        {
            QMessageBox err;
            if(nSample<0)
                err.setText("Can't read number of samples from input files.");
            else
                err.setText("Outgroup should be within selected samples.");

            err.exec();
        }
}
    return;
}

int TreeOptionsDialog::generatePhylipMatrix(QList<VariationPosition *> vpList, bool nonCoding,QStringList samplelist)
{
    TrackContainer * tr = MainWindow::instance->getUI()->tracks;
     Variation * var= tr->getVariation();

        if(!MainWindow::instance->getUI()->tracks->tracklist.isEmpty())
        {
            PCOA pcoa;
            //MainWindow::instance->getUI()->listWidget->selectAll();

            if(pcoa.constructDissimilarityMatrix(var,samplelist,vpList, nonCoding)!=-1)
            {
                return 0;
            }else
            {
                qDebug() << "pcoa.constructDissimilarityMatrix returns -1";
                return -1;
            }

        }else
        {
            QMessageBox msg;
            msg.setText("Please load data in Main Window");
            msg.exec();
            return -1;
        }
}


void TreeOptionsDialog::on_comboBox_method_currentIndexChanged(int index)
{
    if (index==0)
    {
        ui->checkBox_negallowed->setChecked(true);
       // ui->checkBox_negallowed->setEnabled(false);
    }else
    {
        ui->checkBox_negallowed->setChecked(false);
        //ui->checkBox_negallowed->setEnabled(true);
    }
}

void TreeOptionsDialog::GetOptions(TreeCalculation * threadtree)
{
    bool ok;
    int power=ui->lineEdit_power->text().toDouble(&ok);
    int noutgroup=1;
    if (MainWindow::instance->getUI()->chrBox->count()>0)// if data is loaded

        if( MainWindow::instance->selectedItems.size()>0){

         noutgroup =  MainWindow::instance->selectedItems.indexOf(ui->comboBox_sampleList->currentText())+1;
         qDebug()<<"size="<< MainWindow::instance->selectedItems.size()<<"outgroup ="<<noutgroup;
        }
        else{
        noutgroup = MainWindow::instance->getUI()->tracks->getVariation()->samples.indexOf(ui->comboBox_sampleList->currentText())+1;
        }

    threadtree->setParamaters("matrixInPhylipFormat.txt","fitch.txt","tree.tre",
                             noutgroup,ui->checkBox_globalArrange->isChecked(),ui->checkBox_randomSp->isChecked(),
                             ui->comboBox_method->currentIndex(),ui->checkBox_negallowed->isChecked(),0,
                             power,ui->checkBox_replicates->isChecked(),true,true);
    emit optionsSaved(threadtree);
}

void TreeOptionsDialog::on_pushButton_more_clicked()
{
    if (!showMore)
    {
        ui->frame_advancedSet->setEnabled(true);
        this->adjustSize();
        showMore=true;
    }
    else
    {
        ui->frame_advancedSet->setEnabled(false);
        this->adjustSize();
        showMore=false;
    }
}

void TreeOptionsDialog::on_pushButton_ok_clicked()
{

        if(MainWindow::instance->getUI()->tracks->getSequence()){

                // validate Tree options dialog
                if(ui->checkBox_selectedsamples->isChecked()){
                    MainWindow::instance->selectedItems.clear();
                    foreach(QListWidgetItem * item, MainWindow::instance->getUI()->listWidget->selectedItems()){
                        MainWindow::instance->selectedItems.append(item->text());
                    }
                    if(MainWindow::instance->selectedItems.isEmpty())
                    {
                        QMessageBox warning(QMessageBox::Critical, "Selection Error", "No samples selected", QMessageBox::Ok);
                        warning.exec();
                        return;
                    }
                    QStringList selectedList=MainWindow::instance->selectedItems;
                    qDebug()<<"selectedItems"<<selectedList;

                    int num = selectedList.indexOf(ui->comboBox_sampleList->currentText());
                    if(num<0){
                        QMessageBox warning(QMessageBox::Critical, "Outgroup selection error ", "Please choose outgroup within selected samples.", QMessageBox::Ok);
                        warning.exec();
                        return;
                    }
                }
                else{
                    for(int i = 0; i <  MainWindow::instance->getUI()->listWidget->count(); ++i){
                        QListWidgetItem * item= MainWindow::instance->getUI()->listWidget->item(i);
                        MainWindow::instance->selectedItems.append(item->text());
                    }
                }

        }
        else
        {
            qDebug() << "Data is not loaded";
            QMessageBox warning(QMessageBox::Warning, "Tree Computation Error ", "Data is not loaded", QMessageBox::Ok);
            warning.exec();
            return;
        }
        SNPSelectionDialog * ssd = new SNPSelectionDialog(this);
        ssd->pcoa = false;
        ssd->savemode =false;
        ssd->selectedsamples=ui->checkBox_selectedsamples->isChecked();
        connect(ssd, SIGNAL(startTreeComputation(QList<VariationPosition*>,bool,bool)),this,SLOT(computeTree(QList<VariationPosition*>,bool,bool)));
        ssd->show();

}

void TreeOptionsDialog::incrementDone()
{
    this->donesamples++;
    progressdialog->setValue(this->donesamples);

    qApp->processEvents();
    if (progressdialog->wasCanceled())
    {
        //emit signal that it was cancelled, should be catched by treecalculation
        worker->canceltree();
        ui->pushButton_ok->setEnabled(true);
    }
    if(worker->successfullwork)
    {
        //emit finished signal
        progressdialog->hide();
    }
}

void TreeOptionsDialog::viewTree()
{
   // Read the written file as QString and pass to treeviewdialog::showTree()
   QFile source("tree.tre");
   source.open(QIODevice::ReadOnly);
   QString newickString=QString::fromUtf8(source.readAll().constData());
   TreeViewDialog * tvd = new TreeViewDialog(this);
   int from=MainWindow::instance->getUI()->tracks->range.from;
   int to=MainWindow::instance->getUI()->tracks->range.to;
   QString chr = MainWindow::instance->getUI()->chrBox->currentText();
   tvd->showTree(newickString,chr,from,to);
   tvd->show();

   close();
}

void TreeOptionsDialog::fireErrorMessage(QString err)
{
    QString text = err;
    QMessageBox msg;
    msg.setText(text);
    msg.addButton("Return", QMessageBox::YesRole);
    msg.exec();
}


void TreeOptionsDialog::on_comboBox_sampleList_currentIndexChanged(const QString &arg1)
{
    //ui->lineEdit_spNum->setText(QString::number(MainWindow::instance->getUI()->tracks->getVariation()->samples.indexOf(arg1)));
    qDebug() << "Outgroup species is " << arg1;
}

void TreeOptionsDialog::on_pushButton_display_tree_clicked()
{
   QByteArray bOutTree = ui->lineEdit_treeFile->text().toLatin1();
   char * cOutTree = bOutTree.data();
   // Read the written file as QString and pass to treeviewdialog::showTree()
   QFile source(cOutTree);
   source.open(QIODevice::ReadOnly);
   QString fileString=QString::fromUtf8(source.readAll().constData());
   source.close();

   // check if the file is nexus
   QRegExp nexus_comment(QRegExp::escape("#nexus"));
   nexus_comment.setCaseSensitivity(Qt::CaseInsensitive);
   QString newickString;
   int ind=nexus_comment.indexIn(fileString);
   qDebug()<<fileString<<"ind="<<ind<<"\n";
   if(nexus_comment.indexIn(fileString)>=0){
       qDebug()<<"nexus file detected\n";
       // parse nexus file
       std::string str=nexusutils::get_newickstring(fileString.toStdString());
       newickString=QString::fromStdString(str);
       QRegExp nexus_extended_comments(QRegExp::escape("[")+".+"+QRegExp::escape("]"));
       nexus_extended_comments.setMinimal(true);
       newickString.remove(nexus_extended_comments);
   }
   else{
       newickString=fileString;
       QRegExp nexus_extended_comments(QRegExp::escape("[")+".+"+QRegExp::escape("]"));
       nexus_extended_comments.setMinimal(true);
       newickString.remove(nexus_extended_comments);
   }
   // if there isn't an ending semicolon add it please
   QRegExp ending_semicolon("[;][\s]*");
   if(newickString.indexOf(ending_semicolon)<0){
       newickString.append(";");
   }
   qDebug() << "newickString before parsing "<<newickString<<"\n";
   QString newickString_beforeparsing=newickString;
   outnewickfile=fopen("normalisednewick.txt", "w");
   parserrorfile=fopen("parseerror.txt", "w");
   // validate newick string
   // Insert the string into the input stream.
   YY_BUFFER_STATE bufferState = yy_scan_string(newickString.toUtf8().constData());
   // Parse the string.
   int parse_status=yyparse();
   qDebug()<<"parse status"<<parse_status<<"<\n";
   // flush the input stream.
   yy_delete_buffer(bufferState);
   fclose(outnewickfile);
   fclose(parserrorfile);

   if(parse_status==0){
    // Read the written file as QString and pass to treeviewdialog::showTree()
  QFile nsource("normalisednewick.txt");
  nsource.open(QIODevice::ReadOnly);
  newickString=QString::fromUtf8(nsource.readAll().constData());
  qDebug()<<"final newickstring "<<newickString;
    TreeViewDialog * tvd = new TreeViewDialog(this);
    int from=MainWindow::instance->getUI()->tracks->range.from;
    int to=MainWindow::instance->getUI()->tracks->range.to;
    QString chr = MainWindow::instance->getUI()->chrBox->currentText();
    tvd->showTree(newickString,chr,from,to);
    tvd->show();
    close();
    nsource.close();
   }
   else{
       // read parse error file
       QFile esource("parseerror.txt");
       esource.open(QIODevice::ReadOnly);
       QString errorString=QString::fromUtf8(esource.readAll().constData());
       QRegExp tab(QRegExp::escape("\t"));
       QRegExp colon(QRegExp::escape(":"));
       QStringList elist=errorString.split(tab);
       QStringList loc=elist[0].split(colon);
       int pos=loc[1].toInt();
       newickString_beforeparsing.insert(pos-1,'^');
       qDebug()<<"ERROR "<<errorString;
       QMessageBox warning(QMessageBox::Critical, "Problem parsing tree file", "Problem parsing tree file "+ui->lineEdit_treeFile->text().toLatin1()+"\nError near position "+loc[1]+"\t"+elist[2].split(colon)[1]+"\t"+newickString_beforeparsing+"\n Please select a valid nexus or newick format tree file.", QMessageBox::Ok);
       warning.exec();
       esource.close();
       return;
   }
}

void TreeOptionsDialog::on_pushButton_save_clicked()
{
    if(MainWindow::instance->getUI()->tracks->getSequence())
        qDebug() << "Data is loaded, saving alignment...";
    else
    {
        qDebug() << "Data is not loaded";
        QMessageBox warning(QMessageBox::Warning, "Save Alignment Error ", "Data is not loaded", QMessageBox::Ok);
        warning.exec();
        return;
    }
    SNPSelectionDialog * ssd = new SNPSelectionDialog(this);
    ssd->pcoa = false;
    ssd->savemode =true;
    ssd->selectedsamples=ui->checkBox_selectedsamples->isChecked();
    ssd->changeButtonText();
    connect(ssd, SIGNAL(startSaveAlignment(QList<VariationPosition*>,bool,bool)),this,SLOT(saveAlignment(QList<VariationPosition*>,bool,bool)));
    ssd->show();
}

QList<QString> TreeOptionsDialog::leftalign(QString r, QString s){
    int rlength=r.length();
    int slength=s.length();
    QString ar=r;
    QString as=s;
    if(slength>rlength){
    // insertion
        ar="";
        //qDebug()<<"insertion";
        int j=0;
        for (int i=0;i<slength;i++){
            QChar sbase=s.at(i);
            QChar rbase=r.at(j);
            if(sbase==rbase){
                ar=ar.append(rbase);
                j++;
            }
            else{
                ar=ar.append("-");
            }
         }
    }
    if(slength<rlength){
    // deletion
        as="";
        //qDebug()<<"deletion";
        int j=0;
        for (int i=0;i<rlength;i++){
            QChar rbase=r.at(i);
            QChar sbase=s.at(j);
            if(sbase==rbase){
                as=as.append(sbase);
                j++;
            }
            else{
                as=as.append("-");
            }
         }
    }
    if(slength==rlength){
    // snp
        //qDebug()<<"snp";
        ar=r;
        as=s;

    }

    QList <QString> retlist;
    retlist<<ar<<as;
    return retlist;
 }

QList<int> TreeOptionsDialog::getEdit(QString ref){
    int pos=-1;
    int len=0;
    for (int i=0;i<ref.length();i++){
        if(ref.at(i)=='-'){
            if(pos==-1) {pos=i;}
            len++;
        }
    }
    QList<int> ret;
    ret.append(pos);
    ret.append(len);
    return ret;
}

QString TreeOptionsDialog::applyedit(QString ref,QList<int> edits){
    int pos=edits[0];
    int len=edits[1];
    for(int i=0;i<len;i++){
        ref.insert(pos,"-");
    }
    return ref;
}

QList<int> TreeOptionsDialog::translateEdit(QList<int> originaledit,QList<int> appliededit){
    int opos=originaledit[0];
    int olen=originaledit[1];
    int apos=appliededit[0];
    int alen=appliededit[1];
    if(apos==-1){
        return originaledit;
    }
    else{
        if(apos<=opos){
            QList<int> newedit;
            newedit.append(opos+alen);
            newedit.append(olen);
            return newedit;
        }
        else{
            return originaledit;
        }
    }
}

QList<QString> TreeOptionsDialog::leftMSA(QString ref,QList <QString> samples){

    int nos=samples.size();
    //qDebug()<<"sample nos="<<nos;
    QList <QString> result;
    QList <QList <int> > edits;
    // do pairwise alignments with ref and get edits
    for (int i=0;i<nos;i++){
        //qDebug()<<ref<<":"<<samples[i];
        QList <QString> res=leftalign(ref,samples[i]);
        edits.append(getEdit(res[0]));
        result.append(res[1]);
        //qDebug()<<result[i];
        //qDebug()<<edits[i];
     }

    QList <QString> editedsamples;
    // initialize editedsamples with samples
    for(int i=0;i<nos;i++){
        editedsamples.append(result[i]);
    }
    //iteration i apply e[i] on s[j] where i!=j;
    for (int i=0;i<nos;i++){
        for(int j=0;j<nos;j++){
            if(i!=j){
                editedsamples.replace(j,applyedit(editedsamples[j],edits[i]));
            }
        }
        for(int j=i+1;j<nos;j++){
            edits.replace(j,translateEdit(edits[j],edits[i]));
        }
    }

    //qDebug()<<"\n"<<ref;
    /*for(int i=0;i<nos;i++){
        qDebug()<<"\n"<<editedsamples[i];
    }
    */
    return editedsamples;

}

void TreeOptionsDialog::on_pushButton_choosedirectory_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose location to save alignment"),
                                                    ui->lineEdit_alignFile->text(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if (!(dir.isNull() || dir ==ui->lineEdit_alignFile->text())) {
        ui->lineEdit_alignFile->setText(dir);
        ui->pushButton_save->setEnabled(true);
    }

}

void TreeOptionsDialog::on_pushButton_choosefile_clicked()
{
    QString f = QFileDialog::getOpenFileName(this, tr("Select precomputed tree file (nexus/newick format)"), ui->lineEdit_treeFile->text(), tr("All Files (*)"));
    if (!(f.isNull() || f ==ui->lineEdit_treeFile->text())) {
        ui->lineEdit_treeFile->setText(f);
        ui->pushButton_display_tree->setEnabled(true);
    }
}
