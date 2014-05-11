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
#ifndef GROUPMANAGER_H
#define GROUPMANAGER_H

#include <QDialog>
#include "trackcontainer.h"
#include "variation.h"
#include "dbquery.h"

namespace Ui {
    class GroupManager;
}

class GroupManager : public QDialog {
    Q_OBJECT
    
public:
    explicit GroupManager(QWidget *parent = 0);
    ~GroupManager();
    
private slots:
    void on_addGroupButton_clicked();
    void updateGroupCombo();
    void updateSubGroupCombo();
    void on_groupCombo_currentIndexChanged(const QString &arg1);
    void on_removeSelectedButton_clicked();
    void on_removeAllButton_clicked();
    void on_addSelectedButton_clicked();
    void on_doneButton_clicked();

    void on_pushButton_color_clicked();

    void on_pushButton_deleteGroup_clicked();

    void on_subGroupCombo_currentIndexChanged(const QString &arg1);

private:
    Ui::GroupManager *ui;
    TrackContainer *area;
    Variation *var;
    QString group;
    DBQuery dbq;

    void updateLists();
};

#endif // GROUPMANAGER_H
