#ifndef COLORS_H
#define COLORS_H

#include <QDialog>

namespace Ui {
class Colors;
}

class Colors : public QDialog {
    Q_OBJECT
    
public:
    explicit Colors(QWidget *parent = 0);
    ~Colors();
    
private slots:
    void on_pushButton_clicked();

private:
    Ui::Colors *ui;
};

#endif // COLORS_H
