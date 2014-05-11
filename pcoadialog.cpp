#include "pcoadialog.h"
#include "ui_pcoadialog.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPointer>
#include "pcoa.h"

PCOADialog::PCOADialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PCOADialog)
{
    areas= ((MainWindow*)parent)->getUI()->tracks;
    vars = areas->getVariation();

    ui->setupUi(this);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    //scene = new QGraphicsScene(this);
    scene = new GridScene;
    scene_x = new QGraphicsScene;
    scene_y = new QGraphicsScene;

    ui->graphicsView->setScene(scene);
    ui->graphView_xax->setScene(scene_x);
    ui->graphView_yax->setScene(scene_y);

    ui->graphView_xax->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphView_xax->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphView_yax->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphView_yax->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->graphicsView->resize(QSize(this->geometry().size().width()-18,this->geometry().size().height()-18));
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    connect(ui->graphicsView, SIGNAL(specialZoomIn()), this, SLOT(zoomIn()));
    connect(ui->graphicsView, SIGNAL(specialZoomOut()), this, SLOT(zoomOut()));
    connect(ui->graphicsView, SIGNAL(scroll()), this, SLOT(adjustScrollBar()));
    connect(ui->graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), ui->graphView_xax->horizontalScrollBar(), SLOT(setValue(int)));
    connect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->graphView_yax->verticalScrollBar(), SLOT(setValue(int)));

    panning = false;
}

Ui::PCOADialog *PCOADialog::getUI()
{
    return ui;
}

PCOADialog::~PCOADialog()
{
    delete ui;
    delete scene_x;
    delete scene_y;
    delete scene;
}

void PCOADialog::initData(alglib::real_2d_array fd, bool selectedOnly, QList<QString> selectedSampleNamesList)
{
    this->selectedSampleNamesList= selectedSampleNamesList;
    this->finDa=fd;
    this->selectedOnly = selectedOnly;
    //computing the range of x and y
    min_x=0, max_x=0 , min_y=0, max_y=0;

    for (int s=0;s<fd.cols();s++)
    {
        if (fd[0][s]>= max_x)
        {
            max_x=fd[0][s];
        }
        else if(fd[0][s] <=min_x)
        {
            min_x=fd[0][s];
        }
        if (fd[1][s] >= max_y)
        {
            max_y =fd[1][s];
        }
        else if(fd[1][s]<= min_y)
        {
            min_y = fd[1][s];
        }
    }
    this->finDa = fd;
    currentPortWidth = ui->graphicsView->width();
    currentPortHeight = ui->graphicsView->height();
    ui->textBrowser_snpposition->setText(MainWindow::instance->infoText);

    plotAxis(currentPortWidth, currentPortHeight/*,fd*/);
    plotPoints(fd, xZero, yZero, xScale, yScale, selectedOnly,selectedSampleNamesList );
    showLegend();
}

void PCOADialog::plotAxis(int portwidth, int portheight/*, alglib::real_2d_array fd*/)
{
    int margin = 50;
    int tickGap= 40;
    double length_x =1.1*( max_x-min_x);//lengthen the range such that the max value won't be in the right most or left most
    double length_y =1.1*( max_y-min_y); //same idea

    if (length_y == 0)
        length_y = length_x;

    int xAxisLength= portwidth-margin;
    int yAxisLength= portheight-margin;

    xScale = xAxisLength / length_x;

    yScale = yAxisLength / length_y;

    qreal bottom= portheight-margin;
    qreal left=0;
    qreal right=portwidth-margin;
    qreal top=0;

    if (min_x < 0 && max_x >0)
        xZero = ((0-min_x)+(0.05*(max_x-min_x)))*xScale;
    else if (min_x < 0 && max_x<0)
        xZero = left+((0.05*(max_x-min_x))*xScale);//work on this later, consider if min_x too far away from x=0
    else if (min_x > 0 && max_x>0)
        xZero = right-((0.05*(max_x-min_x))*xScale);

    if (min_y < 0 && max_y >0)
        yZero = (max_y+(0.05*(max_y-min_y)))*yScale;
    else if (min_y <0 && max_y <0)
        yZero = top+((0.05*(max_x-min_x))*yScale);//work on this later, consider if min_x too far away from x=0
    else if (min_y >0 && max_y >0)
        yZero = bottom-((0.05*(max_x-min_x))*yScale);

    QPen grayPen(Qt::gray);
    QPen redPen(Qt::red);
    grayPen.setWidth(2);
    redPen.setWidth(0.5);

    yAxis= scene->addLine(xZero,top,xZero, bottom,redPen);
    xAxis= scene->addLine(left, yZero,  right, yZero,redPen);
    horizontalBorder= scene_x->addLine(left,bottom,right, bottom,grayPen);
    verticalBorder= scene_y->addLine(left, top,  left, bottom,grayPen);

    //plotting ticks for axis
    int xNTick= xAxisLength/tickGap;

    QPointer<QGraphicsTextItem> xTickValue[xNTick];

    int tickPos=0;
    for (int y=0;y<xNTick;y++)
    {
        tickPos = (y+1)*tickGap;

        scene_x->addLine(tickPos,bottom-5,tickPos,bottom, grayPen);

        //put tick values
        qreal value= (tickPos-xZero)/xScale;
        xTickValue[y] = new QGraphicsTextItem;
#ifdef WIN32
        xTickValue[y]->setFont(QFont("Helvetica [Cronyx]",8,QFont::Normal,false));
#endif
#ifdef __APPLE__ && __MACH__
        xTickValue[y]->setFont(QFont("Helvetica [Cronyx]",11,QFont::Normal,false));
#endif
        xTickValue[y]->setPlainText(QString::number((double)value,'f',3));
        xTickValue[y]->setPos(tickPos-10,bottom);
        xTickValue[y]->adjustSize();
        scene_x->addItem(xTickValue[y]);
    }

    int yNTick= yAxisLength/tickGap;
    QPointer<QGraphicsTextItem> yTickValue[yNTick];
    for (int y=0;y<yNTick;y++)
    {
        tickPos = (y+1)*tickGap;
        scene_y->addLine(left,bottom-tickPos,left+5,bottom-tickPos, grayPen);

        //put tick values
        qreal value= (yZero-(bottom-tickPos))/yScale;
        yTickValue[y] = new QGraphicsTextItem;
#ifdef WIN32
        yTickValue[y]->setFont(QFont("Helvetica [Cronyx]",8,QFont::Normal,false));
#endif
#ifdef __APPLE__ && __MACH__
        yTickValue[y]->setFont(QFont("Helvetica [Cronyx]",11,QFont::Normal,false));
#endif
        yTickValue[y]->setPlainText(QString::number((double)value,'f',3));
        yTickValue[y]->setPos(left-margin,bottom-tickPos-10);
        yTickValue[y]->adjustSize();
        scene_y->addItem(yTickValue[y]);
    }
}

void PCOADialog::plotPoints(alglib::real_2d_array fd, qreal xZero, qreal yZero, qreal xScale, qreal yScale, bool selectedOnly, QList<QString> selectedSampleNamesList)
{
    QPointer<Point> dot[fd.cols()];
    QString groupName[fd.cols()];
    int groupID[fd.cols()];
    int sampleIndex[fd.cols()];
    int index=0, sideSampleIndex=0;
    foreach (QString gp, vars->groups.keys()) {

        for (long gpi = 0; gpi < vars->groups[gp].size(); gpi++) {
            if(!selectedOnly)
            {
                groupName[vars->groups[gp][gpi]]= gp;
                groupID[vars->groups[gp][gpi]]=index;
            }else
            {
                if (selectedSampleNamesList.contains(vars->samples[vars->groups[gp][gpi]]))
                {
                    sampleIndex[sideSampleIndex]=vars->groups[gp][gpi];
                    groupName[sideSampleIndex]=gp;
                    groupID[sideSampleIndex]=index;
                    sideSampleIndex++;
                }
            }
        }
        index++;
    }

    QColor pointColor;
    QMap<QString, QStringList> overlap;

    for (int j=0;j<fd.cols();j++)
    {

        dot[j] = new Point(xZero+(fd[0][j]*xScale),yZero+(fd[1][j]*(-1*yScale)));

        scene->addItem(dot[j]);
        dot[j]->xval=fd[0][j];
        dot[j]->yval=fd[1][j];

        dot[j]->groupName = groupName[j];
        dot[j]->groupID = groupID[j];
        pointColor = vars->groupColor[groupName[j]];
        dot[j]->setColor(pointColor);//groupID should be assigned first before setting the color, work on this later
        if(selectedOnly)
        {
            dot[j]->sampleID = sampleIndex[j];
            dot[j]->sampleName = vars->samples[sampleIndex[j]];
        }else
        {
            dot[j]->sampleID = j;
            dot[j]->sampleName = vars->samples[j];
        }

        QString x= QString::number(xZero+(fd[0][j]*xScale));
        QString key = x.append(QString::number(yZero+(fd[1][j]*(-1*yScale))));

        if(overlap.contains(key))
            overlap[key].append(dot[j]->sampleName);
        else
        {
            QStringList snameList;
            snameList.append(dot[j]->sampleName);
            overlap.insert(key,snameList);
        }
    }

    QString text;
    if (selectedOnly)
    {
        text.append("Selected Sample: \n") ;

        //appending selectedSampleNameList
        foreach(QString sample, selectedSampleNamesList)
            text.append(sample+" ");

        text.append("\n");
    }else
        text.append("All samples selected\n");

    foreach(QStringList sampleList, overlap)
    {

        if(sampleList.size()>1)
        {
            text.append("\nFully Overlap : \n - ");
            int i=0;
            foreach(QString name, sampleList)
            {
                i++;
                if(i==sampleList.size())
                    text.append(name+"\n\n");
                else
                    text.append(name+", ");
            }
        }
    }

     ui->textBrowser_overlap->setText(text);
}

void PCOADialog::showLegend()
{
    QList<int> selectedIndex;
    for (int s=0;s<vars->samples.size();s++)
    {
        if(PCOA::isSelectedSample(vars->samples[s],this->selectedSampleNamesList))
        {
            selectedIndex.append(s);
        }
    }

    foreach (QString gp, vars->groups.keys()) {
        foreach(int i, selectedIndex)
        {
            if(vars->groups[gp].contains(i))
            {
                QListWidgetItem * item = new QListWidgetItem;
                item->setText(gp);
                item->setTextColor(vars->groupColor[gp]);
                item->setTextAlignment(Qt::AlignCenter);
                item->setFlags(Qt::NoItemFlags);
                ui->listWidget->addItem(item);
                break;
            }
        }
    }
}

QTextBrowser * PCOADialog::getTextBrowser1()
{
    return ui->textBrowser1;
}

QTextBrowser * PCOADialog::getTextBrowser2()
{
    return ui->textBrowser2;
}

void PCOADialog::zoomIn()
{
    //ui->graphicsView->scene()->clear();
    scene = new GridScene;
    scene_x = new QGraphicsScene;
    scene_y = new QGraphicsScene;

    currentPortWidth = 1.5 * currentPortWidth;
    currentPortHeight = 1.5 * currentPortHeight;

    plotAxis(currentPortWidth,currentPortHeight/*,this->finDa*/);
    plotPoints(finDa, xZero, yZero, xScale, yScale, selectedOnly,selectedSampleNamesList);

    ui->graphicsView->setScene(scene);
    ui->graphView_xax->setScene(scene_x);
    ui->graphView_yax->setScene(scene_y);

    adjustScrollBar();
    ui->graphicsView->highlighted=false;
    ui->graphicsView->pointclicked=false;
}

void PCOADialog::zoomOut()
{
    double minWidth= 0.5*ui->graphicsView->width();
    double minHeight= 0.5*ui->graphicsView->height();
    if(currentPortWidth>=minWidth && currentPortHeight>=minHeight)
    {
        ui->graphicsView->scene()->clear();
        scene = new GridScene;
        scene_x = new QGraphicsScene;
        scene_y = new QGraphicsScene;

        currentPortWidth = .75 * currentPortWidth;
        currentPortHeight = .75 * currentPortHeight;

        plotAxis(currentPortWidth,currentPortHeight/*,this->finDa*/);
        plotPoints(finDa, xZero, yZero, xScale, yScale, selectedOnly,selectedSampleNamesList);

         ui->graphicsView->setScene(scene);
         ui->graphView_xax->setScene(scene_x);
         ui->graphView_yax->setScene(scene_y);

         adjustScrollBar();
         //ui->graphicsView->centerOn(ui->graphicsView->ms);
    }

}

void PCOADialog::adjustScrollBar()
{

    if(ui->graphicsView->pointclicked)
    {
       //centering on a point
       qreal newx= xZero+(ui->graphicsView->xclick*xScale);
       qreal newy= yZero+(ui->graphicsView->yclick*(-1*yScale));
       ui->graphicsView->centerOn(newx,newy);

    }
    else
    {
        //centering around focus
        qreal newx= xZero+(ui->graphicsView->xaround*xScale);
        qreal newy= yZero+(ui->graphicsView->yaround*(-1*yScale));
        ui->graphicsView->centerOn(newx,newy);
    }
}

void PCOADialog::zoomPlot()
{
    currentPortWidth = 1.5 * currentPortWidth;
    currentPortHeight = 1.5 * currentPortHeight;

    plotPoints(finDa, xZero, yZero, xScale, yScale, selectedOnly,selectedSampleNamesList);
}

void PCOADialog::on_pushButton_save_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Select where to save file..."), "", tr("Images (*.png)"));;

    if (0 == filename.length()) return;
    if (!(filename.endsWith(".png") || filename.endsWith(".jpg") || filename.endsWith(".bmp") || filename.endsWith(".jpeg") || filename.endsWith(".ppm") || filename.endsWith(".tiff") || filename.endsWith(".xbm") || filename.endsWith(".xpm")))
    {
        filename.append(".png");
    }
    QPixmap pm = QPixmap::grabWidget(ui->widget_plotArea);
    //QPixmap pm = QPixmap::grabWidget(MainWindow::instance->getUI()->listWidget);
    pm.save(filename);
}
