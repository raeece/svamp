#ifndef LOOKSEQWINDOW_H
#define LOOKSEQWINDOW_H

#include <QDialog>

namespace Ui {
class LookSeqWindow;
}

class LookSeqWindow : public QDialog
{
    Q_OBJECT
    
public:
    explicit LookSeqWindow(QWidget *parent = 0);

    void locatePosition(int start, QString chromName, QStringList isolateID);
    ~LookSeqWindow();
    
private:
    Ui::LookSeqWindow *ui;
};

#endif // LOOKSEQWINDOW_H
