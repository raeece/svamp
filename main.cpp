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
#include <QtGui/QApplication>
#include "mainwindow.h"
#include "variation.h"

MainWindow* MainWindow::instance = NULL;

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QApplication::setStyle(new QPlastiqueStyle);
    QFile file;
    file.setFileName(":/qss/svamp_mac.qss");

#ifdef WIN32
    file.setFileName(":/qss/svamp_win32.qss");
#endif


    if(!file.open(QFile::ReadOnly))
        qDebug() << file.error();

    a.setStyleSheet(file.readAll());
    MainWindow w;
    w.show();
    return qApp->exec();
}
