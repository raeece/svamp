#include "menu.h"
#include <QMenu>
#include <QMenuBar>

Menu::Menu(QWidget *parent) : QMainWindow(parent)
{
    QAction *quit = new QAction("&Quit", this);

      QMenu *file;
      file = menuBar()->addMenu("&File");
      file->addAction(quit);

      connect(quit, SIGNAL(triggered()), qApp, SLOT(quit()));
}
