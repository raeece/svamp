#include "treeviewdialog.h"
#include "ui_treeviewdialog.h"
#include <QFile>
#include <QDebug>
#include "mainwindow.h"
#include "ui_mainwindow.h"


TreeViewDialog::TreeViewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TreeViewDialog)
{
   track=MainWindow::instance->getUI()->tracks;
   vars = track->getVariation();
   ui->setupUi(this);

}

TreeViewDialog::~TreeViewDialog()
{
    delete ui;
}

void TreeViewDialog::showTree(QString newickString, QString chr,int from, int to){

    this->newickString=newickString;
    QRect rect=MainWindow::instance->geometry();
    int h=rect.height();
    int w=rect.width();
    QString fullHtml=generateHTML(w,h,0);

    ui->webView->setHtml(fullHtml);
    ui->webView->resize(QSize(w,h));
    ui->textBrowser_treeCaption->setText(MainWindow::instance->infoText);

     // uncomment for debuggin the html rendered on webkit
    QString filename ="tree.html";
    QFile file(filename);
    if ( file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream( &file );
        stream << fullHtml << endl;
        file.close();
    }
}

QString TreeViewDialog::generateHTML(int width, int height,int phylotype){

    QFile source1(":html/head.html");
    source1.open(QIODevice::ReadOnly);
    QString headhtml=QString::fromUtf8(source1.readAll().constData());

    QFile source2(":html/foot1.html");
    source2.open(QIODevice::ReadOnly);
    QString foot1html=QString::fromUtf8(source2.readAll().constData());

    QFile source3(":html/foot2.html");
    source3.open(QIODevice::ReadOnly);
    QString foot2html=QString::fromUtf8(source3.readAll().constData());

    QHash<QString, QString> sample_group;
    QHash<QString, QString> group_colour;

    if(vars)
    {
    foreach (QString gp, vars->groups.keys()) {
        QList <int> samples=vars->groups[gp];
        foreach (int sampid, samples){
            QString sample_name=vars->samples[sampid];
            sample_group[sample_name]=gp;
        }
         QColor color= vars->groupColor[gp];
         QString hexcode=color.name();
         group_colour[gp]=hexcode;
     }
    }

    QString phyloxml=newickToXML(newickString,sample_group,group_colour);
    // Count the number of leaf nodes and adjust the height of the tree rendered
    QRegExp leafnode_branch_length ("[(,]([^(,:]+):([-]*[0-9]+[.]*[0-9]*)");
    int pos=0;
    int leaf_nodes_count=0;
    while (pos >= 0) {
        pos = leafnode_branch_length.indexIn(newickString,pos);
        if (pos >= 0) {
            pos += leafnode_branch_length.matchedLength();
            leaf_nodes_count++;    // count the number of leaf nodes
        }
    }
    int ht=25*leaf_nodes_count;
    int height_in_pixels=height;
    if(ht>height){
        height_in_pixels=ht;
    }
    //qDebug()<<"height="<<height<<"ht="<<ht<<"height_in_pixels="<<height_in_pixels<<"leaf_nodes_count="<<leaf_nodes_count<<"\n";
    // no need to adjust height for circular tree
    QString wsize(QString::number(width)+","+QString::number(height));
    if(!phylotype){
    wsize=QString::number(width)+","+QString::number(height_in_pixels);
    }
    QString phylotypestr("");
    if(phylotype){
    phylotypestr=",'circular'";
    }
    QString fullHtml (headhtml+phyloxml+foot1html+wsize+phylotypestr+foot2html);
    return fullHtml;

}

QString TreeViewDialog::newickToXML(QString newick,QHash<QString,QString> sample_group,QHash<QString,QString> group_colour){

    QRegExp leafnode_branch_length ("[(,]([^(,:]+):([-]*[0-9]+[.]*[0-9]*)");
    QRegExp internal_node_branch_length ("[)]([^:]*):([-]*[0-9]+[.]*[0-9]*)");
    QRegExp unwanted ("[,;]");
    QRegExp final_clade_with_name ("[)]([^;]*);");
    QRegExp newick_extended_comments(QRegExp::escape("[")+".+"+QRegExp::escape("]"));


    // remove newick comments [comment about rate etc.,]

    newick_extended_comments.setMinimal(true);
    newick.remove(newick_extended_comments);
    qDebug()<<"newick string after comments removed"<<newick;
    int pos = 0;
    QString xmlheader("<phyloxml xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.phyloxml.org http://www.phyloxml.org/1.10/phyloxml.xsd\" xmlns=\"http://www.phyloxml.org\">");
    xmlheader.append("<phylogeny rooted=\"false\">");
    xmlheader.append("<render>");
    xmlheader.append("<parameters>");
    xmlheader.append("<circular>");
    xmlheader.append(" <bufferRadius>0.5</bufferRadius>");
    xmlheader.append("</circular>");
    xmlheader.append("<rectangular>");
    xmlheader.append("<alignRight>1</alignRight>");
    xmlheader.append("<bufferX>300</bufferX>");
    xmlheader.append("</rectangular>");
    xmlheader.append("</parameters>");
    xmlheader.append("<charts>");
    xmlheader.append("<component type=\"binary\" thickness=\"10\" />");
    xmlheader.append("</charts>");
    xmlheader.append("<styles>");

    QHashIterator<QString, QString> i(group_colour);
     while (i.hasNext()) {
         i.next();
         QString gname=i.key();
         gname = gname.simplified();
         gname.remove(" ");
         xmlheader.append("<"+gname+" fill='"+i.value()+"'"+" stroke='"+i.value()+"'/>");
    }
    xmlheader.append("</styles>");
    xmlheader.append("</render>");
    QString xmlfooter("</phylogeny>");
    xmlfooter.append("</phyloxml>");


    while ((pos = leafnode_branch_length.indexIn(newick, pos)) != -1) {
        QString leaf_name=leafnode_branch_length.cap(1);
        QString branch_length=leafnode_branch_length.cap(2);
        QString xml("");
        QString t_sample_group=sample_group[leaf_name];
        t_sample_group=t_sample_group.simplified();
        t_sample_group.remove(" ");
        xml.append("<clade><name bgStyle='"+t_sample_group+"'>");
        xml.append(leaf_name);
        xml.append("</name><branch_length>");
        xml.append(branch_length);
        xml.append("</branch_length>");
        xml.append("<chart><component>");
        xml.append(t_sample_group);
        xml.append("</component></chart>");
        xml.append("<annotation>");
        xml.append("<desc>"+t_sample_group+"</desc>");
        xml.append("</annotation>");
        xml.append("</clade>\n");
        int l=xml.length();
        pos+=l;
        newick.replace(QRegExp(QRegExp::escape(leaf_name+":"+branch_length)),xml);
     }
    pos= 0;
    while ((pos = internal_node_branch_length.indexIn(newick, pos)) != -1) {
        QString branch_name=internal_node_branch_length.cap(1);
        QString branch_length=internal_node_branch_length.cap(2);
        QString xml("");
        xml.append("<name bgStyle='group1'>");
        xml.append(branch_name);
        xml.append("</name>");
        xml.append("<branch_length>");
        xml.append(branch_length);
        xml.append("</branch_length>");
        xml.append("</clade>\n");
        int l=xml.length();
        pos+=l;
        newick.replace(QRegExp(QRegExp::escape(")"+branch_name+":"+branch_length)),xml);
     }

    pos=0;
    pos=final_clade_with_name.indexIn(newick,pos);
    QString root_name=final_clade_with_name.cap(1);
    QString xml("");
    xml.append("<name bgStyle='group1'>");
    xml.append(root_name);
    xml.append("</name>");
    xml.append("<branch_length>0.0</branch_length>");
    xml.append("</clade>\n");
    newick.replace(final_clade_with_name,xml);
    newick.replace(QRegExp(QRegExp::escape("(")),"<clade>");
    newick.replace(QRegExp(QRegExp::escape(")")),"</clade>");
    newick.remove(unwanted);

    QString fullxml(xmlheader+newick+xmlfooter);
    return fullxml;
}

void TreeViewDialog::on_checkBox_stateChanged(int arg1)
{
    QRect rect=MainWindow::instance->geometry();
    //QRect rect=parentWidget()->parentWidget()->geometry();
    int h=rect.height();
    int w=rect.width();
    QString fullHtml=generateHTML(w,h,arg1);
    ui->webView->setHtml(fullHtml);

     // uncomment for debuggin the html rendered on webkit
    QString filename ="tree.html";
    QFile file(filename);
    if ( file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream stream( &file );
                stream << fullHtml << endl;
                file.close();
            }

}

void TreeViewDialog::on_spinBox_valueChanged(int zoom)
{
#if QT_VERSION >= 0x040500
    ui->webView->setZoomFactor(zoom/100.0);
#else
    ui->webView->setTextSizeMultiplier(zoom/100.0);
#endif

}

void TreeViewDialog::on_pushButton_save_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Select where to save image file..."), "", "");

    if (0 == filename.length()) return;
    if (!(filename.endsWith(".png") || filename.endsWith(".jpg") || filename.endsWith(".bmp") || filename.endsWith(".jpeg") || filename.endsWith(".ppm") || filename.endsWith(".tiff") || filename.endsWith(".xbm") || filename.endsWith(".xpm")))
    {
        filename.append(".png");
    }    QString label = ui->label_region->text();
    QString region = ui->textBrowser_treeCaption->toPlainText();

    QPixmap treePM = QPixmap::grabWidget(ui->webView);
    QPixmap labelPM = QPixmap::grabWidget(ui->label_region);

    QPainter p;
    QFont font;
    font.setPixelSize(ui->spinBox_fontsize->value());
    font.setFamily("Arial");

    int labelWidth= labelPM.width();
    int labelHeight = labelPM.height();

    int treeWidth = treePM.width();
    int treeHeight = treePM.height();

    QPixmap res( treeWidth, treeHeight+labelHeight+1);
    res.fill(Qt::white);

    p.begin( &res );

    p.setFont(font);

    p.drawText(QRect(0,0,labelWidth+16, labelHeight),0,label);

    p.drawPixmap(0,labelHeight, treePM );

    p.drawText(QRect(labelWidth+12,0, treeWidth-(labelWidth+8), labelHeight),Qt::TextWordWrap | Qt::TextDontClip,region);

    p.end();

    res.save(filename);
}
