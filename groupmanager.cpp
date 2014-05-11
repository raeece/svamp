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
#include "groupmanager.h"
#include "ui_groupmanager.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStringListModel>
#include <QDebug>

GroupManager::GroupManager(QWidget *parent) : QDialog(parent), ui(new Ui::GroupManager) {
    ///qDebug() << "GroupManager::GroupManager";
    ui->setupUi(this);
    group = "";
    area = ((MainWindow*)parent)->getUI()->tracks;
    var = area->getVariation();
    updateGroupCombo();
    updateLists();
    setWindowTitle("SVAMP : Manage groups");
    setTabOrder(ui->newGroupLine,ui->addGroupButton);
    setTabOrder(ui->addGroupButton,ui->groupCombo);
    this->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint| Qt::CustomizeWindowHint);
    dbq.connect();
}

GroupManager::~GroupManager() {
    dbq.closeDB();
    delete ui;
}

void GroupManager::updateLists() {
    ///qDebug() << "GroupManager::updateLists";
    bool isGroups = true;

    isGroups = (0 != ui->groupCombo->count());
    ui->groupCombo->setEnabled(isGroups);
    ui->groupList->setEnabled(isGroups);
    ui->subGroupCombo->setEnabled(isGroups);

    ui->addSelectedButton->setEnabled(isGroups);
    ui->removeSelectedButton->setEnabled(isGroups);
    ui->removeAllButton->setEnabled(isGroups);

    ui->groupList->clear();
    ui->ungroupedList->clear();

    if (var) {
        if (var->groups.contains(ui->groupCombo->currentText())) {


            QStringList sampleList = dbq.querySampleList(ui->groupCombo->currentText(), ui->subGroupCombo->currentText());

            int y=0;
            foreach(QString sample, sampleList)
            {
                ui->groupList->addItem(sample);
                ui->groupList->item(y)->setForeground(var->groupColor[ui->groupCombo->currentText()]);
                y++;
            }
        }
        int x=0;

        foreach (int i, var->groups["Ungrouped"]) {
            ui->ungroupedList->addItem(var->samples[i]);
            ui->ungroupedList->item(x)->setForeground(var->groupColor["Ungrouped"]);
            x++;
        }
    }
}

void GroupManager::on_addGroupButton_clicked() {
    ///qDebug() << "GroupManager::on_addGroupButton_clicked";
    QString gp = ui->newGroupLine->text().trimmed();
    QString subgp = ui->cityLine->text().trimmed();

    if(gp.isEmpty()) // check if new group name is empty
    {
        QMessageBox warning(QMessageBox::Warning, "New Group Error ", "Group Name can't be empty", QMessageBox::Ok);
        warning.exec();
        return;
    }else if(subgp.isEmpty())
    {
        QMessageBox warning(QMessageBox::Warning, "New Group Error ", "SubGroup Name can't be empty", QMessageBox::Ok);
        warning.exec();
        return;
    }
    else//capitalizing first character
    {
        int x=0;
        gp[x] = gp[x].toUpper();
        int i = gp.indexOf(" ", x);
        do
        {
            if(i>0)
            {
                gp[i+1]= gp[i+1].toUpper();
            }
            x=i+1;
            i = gp.indexOf(" ", x);
        }while(x<gp.length() && i>0);

        int y=0;
        subgp[y] = subgp[y].toUpper();
        int j = subgp.indexOf(" ", y);
        do
        {
            if(j>0)
            {
                subgp[j+1]= subgp[j+1].toUpper();
            }
            y=j+1;
            j = subgp.indexOf(" ", y);
        }while(y<subgp.length() && j>0);
    }

    int status=0;

    if (!dbq.isConnected()) // database doesn't exist and/or doesn't open
    {
        int lastOccurence = var->source_file.lastIndexOf('/');
        uint timestamp = QDateTime::currentDateTime().toTime_t();
        QString dbname = var->source_file.mid(lastOccurence+1) + QString::number(timestamp)+ ".db";

        QMessageBox warning(QMessageBox::Information, "New Database ", "No database was given. Temporary database ("+dbname+") will be created to save grouping information", QMessageBox::Ok);
        warning.exec();
        if(dbq.createNewDB(dbname))
        {
            dbq.createTables();
        }
    }

    if (var && var->groups.contains(gp)) { // if group name already exist, check if its subgroup exists (or comment this and put constraint in db)
        qDebug() << "Group name exist, checking if subgroup already exists...";
        if(!dbq.isUnique(gp, subgp))
        {
            QMessageBox warning(QMessageBox::Critical, "New Group Error ", "Location already exists.", QMessageBox::Ok);
            warning.exec();
            return;
        }
    }

    //insert into db
    status = dbq.insertIntoLocation(subgp,gp, ui->lineEdit_lat->text().toDouble(), ui->lineEdit_lon->text().toDouble());

    if(status<0)//  error(s) occured
    {
        QMessageBox warning(QMessageBox::Critical, "New Group Error ", "Error creating new group, please check name,latitude, and longitude", QMessageBox::Ok);
        warning.exec();
    }
    else //if insertion to db successfull
    {
        if (var && ! var->groups.contains(gp)) {
            group = gp;
            var->groups[group];//create a new group

            ui->groupCombo->addItem(group);
            ui->groupCombo->setCurrentIndex(ui->groupCombo->count() - 1);

            MainWindow::instance->addGroup(group);
        }else // if new group already exist and new subgroup created instead
        {
            int index= ui->groupCombo->findText(gp);
            if(index!=-1 && index!=ui->groupCombo->currentIndex())
            {
                ui->groupCombo->setCurrentIndex(index);
                ui->subGroupCombo->setCurrentIndex(ui->subGroupCombo->findText(subgp));
            }else
            {
                on_groupCombo_currentIndexChanged(gp);
                ui->subGroupCombo->setCurrentIndex(ui->subGroupCombo->findText(subgp ));
            }
        }

        ui->cityLine->clear();
        ui->newGroupLine->clear();
    }
    //dbq.closeDB();
}

void GroupManager::updateGroupCombo()
{
    ui->groupCombo->clear();
    if (var && 0 == ui->groupCombo->count()) {
        foreach (QString gp, var->groups.keys()) {
            if ("Ungrouped" != gp) ui->groupCombo->addItem(gp);
            //if (group == gp) ui->groupCombo->setCurrentIndex(ui->groupCombo->count() - 1);
        }
        ui->groupCombo->setCurrentIndex(0);
    }
}

void GroupManager::updateSubGroupCombo()
{
    ui->subGroupCombo->clear();
    QStringList cityList = dbq.querySubGroupList(ui->groupCombo->currentText());

    for(int i=0;i<cityList.size();i++)
    {
        ui->subGroupCombo->addItem(cityList.at(i));
    }
}

void GroupManager::on_groupCombo_currentIndexChanged(const QString &g) {

    ui->subGroupCombo->clear();
    if ("" != g && group != g) group = g;
    updateSubGroupCombo();

    //updateLists();
}

void GroupManager::on_removeSelectedButton_clicked() {
    ///qDebug() << "GroupManager::on_removeSelectedButton_clicked";
    QStringList sampleList;
    QList<QListWidgetItem*> selected = ui->groupList->selectedItems();
    if (var && var->groups.contains(group)) {
        foreach (QListWidgetItem* i, selected) {           
            sampleList.append(i->text());

            int index = var->samples.indexOf(i->text());
            if (-1 != index) {
               var->groups["Ungrouped"].append(index);
               var->groups[group].removeOne(index);
            }
        }
    }

    //update in DB
    dbq.updateGroupingInDB(sampleList , "Ungrouped", "Ungrouped");
    updateLists();
}

void GroupManager::on_removeAllButton_clicked() {
    ///qDebug() << "GroupManager::on_removeAllButton_clicked";
    QStringList sampleList;
    if (var && var->groups.contains(group)) {
        QListWidgetItem* i = ui->groupList->takeItem(0);
        while (0 != i) {
            sampleList.append(i->text());
            int index = var->samples.indexOf(i->text());
            if (-1 != index) {
               var->groups["Ungrouped"].append(index);
               var->groups[group].removeOne(index);
            }
            i = ui->groupList->takeItem(0);
        }

    }

    dbq.updateGroupingInDB(sampleList,"Ungrouped", "Ungrouped");
    updateLists();
}

void GroupManager::on_addSelectedButton_clicked() {
    ///qDebug() << "GroupManager::on_addSelectedButton_clicked";
    QStringList sampleList;

    QList<QListWidgetItem*> selected = ui->ungroupedList->selectedItems();
    if (var && var->groups.contains(group)) {
        foreach (QListWidgetItem* i, selected) {

            sampleList.append(i->text());

            int index = var->samples.indexOf(i->text());
            if (-1 != index) {
               var->groups["Ungrouped"].removeOne(index);
               var->groups[group].append(index);
            }
        }
    }

    dbq.updateGroupingInDB(sampleList,ui->groupCombo->currentText(), ui->subGroupCombo->currentText());
    updateLists();
}

void GroupManager::on_doneButton_clicked() {
    //qDebug() << "GroupManager::on_doneButton_clicked";

    area->changeGroups();
    close();
}

void GroupManager::on_pushButton_color_clicked()
{
    QColor color = QColorDialog::getColor(Qt::yellow,this );
    if(color.isValid())
    {
        if(var->groups.contains(ui->groupCombo->currentText()))
        {
            var->groupColor[ui->groupCombo->currentText()] = color;
        }
    }
    updateLists();
    MainWindow::instance->updateGroupWidget();

    //store group coloring to file

    QFile file("groupscolors.pv");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream fout(&file);
        foreach(QString key, var->groupColor.keys())
        {
            fout << key << "\t" << var->groupColor[key].name() << endl;
        }

        file.close();
    }
    //close();
}

void GroupManager::on_pushButton_deleteGroup_clicked()
{
    QMessageBox msgb;
    QString comboText= ui->groupCombo->currentText();
    int groupIndex= ui->groupCombo->currentIndex();
    if(comboText=="")
    {
        msgb.setText("No group selected");
        msgb.exec();
    }
    if(var->groups.contains(comboText))
    {
        QString text = "Discard ";
        text.append(comboText+ "? ");
        msgb.setInformativeText(text);
        msgb.setStandardButtons(QMessageBox::Discard | QMessageBox::Cancel );
        msgb.setDefaultButton(QMessageBox::Cancel);
        int ret= msgb.exec();

        switch (ret) {
        case QMessageBox::Discard:
        {
            QList<int> sampleList = var->groups[comboText];
            //move items to Ungrouped
            foreach(int sampleIndex,sampleList)
            {
                if (-1 != sampleIndex) {
                    var->groups["Ungrouped"].append(sampleIndex);
                    var->groups[comboText].removeOne(sampleIndex);
                }
            }
            //delete from groups' QMap
            var->groups.remove(comboText);

            ui->groupCombo->removeItem(groupIndex);

            //update group widget in MainWindow :
            MainWindow::instance->updateGroupWidget();

            //delete color assignment
            var->groupColor.remove(comboText);

            dbq.deleteGroup(comboText);

            break;
        }
        case QMessageBox::Cancel:
            qDebug() << "Cancel was clicked";
            break;
        default:
            // should never be reached
            break;
        }

    }//end of if
}

void GroupManager::on_subGroupCombo_currentIndexChanged(const QString &arg1)
{
    updateLists();
}
