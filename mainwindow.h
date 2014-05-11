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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStatusBar>
#include <QList>
#include <QWebView>
#include <QCheckBox>
#include <QSignalMapper>
#include <QtGui>
#include "feedback.h"
#include "log.h"
#include "track.h"
#include "tracksequence.h"
#include "trackannotation.h"
#include "trackvariation.h"
#include "trackgraph.h"
#include "dbquery.h"
#include "jbrowsesettingdialog.h"
#include "snpselectiondialog.h"

class SearchResult;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Ui::MainWindow* getUI () { return ui; }
    void setRange (QString chr, long from, long to, long padding = 0);
    void loadData(QString s, QString a, QString v, QString f, QString d);
    void updateCounts(int nonCoding, int nonSynonymous, int synonymous, int indel, int common);
    void updateLegend();
    void updateGroupWidget();

//    void computePcoaViewedSnps(bool all);
//    void computePcoaSelected();
    void runPcoa(bool allSamples, QList<VariationPosition*>  vpList, bool indels);

    int clickedPosition, hoveredPosition;
    QString dbPath;
    long height, top, label_offset;

    static MainWindow* instance;
    static void outOfMemory(QString msg);
    void showProps (QString s);
    void stickyProps (bool sticky);
    bool isNonCoding();
    bool isSynonymous();
    bool isNonSynonymous();
    bool isIndel();
    bool isCommon();
    bool isGroup(QString group);
    void addGroup(QString group);
    void keyPressEvent(QKeyEvent *key);
    void exportView(QString filename);

    Feedback feedback;
    QString colors[6];
    void updateVariationTrack();
    bool coveragePainting;
    QString infoText;

    QColor assignColor(int groupID);
    QStringList selectedItems;//for selected sample

protected:
    void changeEvent(QEvent *e);
    QList <SearchResult*> results;
    bool adjusting;

private:
    void setMacUp();
    Ui::MainWindow *ui;
    int visible_range, totalSNP;
    QString last_props;
    bool props_are_sticky, loading;
    QMap<QString, QCheckBox*> groups;


    QAction *load, *save, *log, *tree, *pcoaall,*pcoaselected, *viewuncovered,
            *mapaction, *about, *grouping, *colourcode ,*allelemapact, *jbrowseSetting, *blues, *greens, *limes;

    QMenu *file, *tools, *pcoa, *uncovered, *edit, *style;

private slots:
//    void on_props_linkClicked(const QUrl &);

    void on_chrBox_textChanged(QString);
    void on_chrBox_currentIndexChanged(QString);
    void on_rsdButton_clicked();
    void on_lineTo_returnPressed();
    void on_lineFrom_returnPressed();
    void on_goButton_clicked();
    void on_findResults_currentRowChanged(int currentRow);
    void on_findClearButton_clicked();
    void on_findButton_clicked();
    void on_findQuery_returnPressed();
    void on_zoomSlider_valueChanged(int value);
    void on_positionSlider_valueChanged(int value);
    void loadFilesAction();
    void saveAction();
    void on_settingsButton_clicked();
    void on_qualitySlider_valueChanged(int value);
    void on_readDepthSlider_valueChanged(int value);
    void on_browserSizeSlider_valueChanged(int value);
    void on_indelCheckBox_clicked();
    void on_synonymousCheckBox_clicked();
    void on_nonSynonymousCheckBox_clicked();
    void track_checkbox_clicked();
    void viewGroupManager();
    void on_ungroupedCheck_clicked();
    void on_nonCodingCheckBox_clicked();
    void on_lineQuality_editingFinished();
    void on_lineDepth_editingFinished();
    void on_lineSize_editingFinished();
    void viewAboutPage();
    void editColour();
    void on_positionLeftButton_clicked();
    void on_positionRightButton_clicked();
    void viewLog();
    void viewUncovered();

    void viewMap();
    void viewAlleleMap();
    void on_lookSeqPushButton_clicked();

    void createMenu();
    void disableDataMenu();
    void enableDataMenu();
    void enableTreeMenuOnly();

    void applyInitialGrouping();
    void launchJBrowseSetting();
    void launchSNPForPCOA();
    void launchSNPForPCOASelected();

    void clearMenuPointers();
    void switchToGreen();
    void switchToBlue();
    void switchToLime();

    int locatePosition(int start, QString chromName, QStringList isolateID);
    QList<long> getVarPosBetween(long x1, long x2);
    void launchTreeComputation();

    void showDetails();

    void on_commonCheckBox_clicked();
};

#endif // MAINWINDOW_H
