#ifndef FEEDBACK_H
#define FEEDBACK_H

#include <QDialog>

namespace Ui {
class Feedback;
}

class Feedback : public QDialog {
    Q_OBJECT
    
public:
    explicit Feedback(QWidget *parent = 0);
    ~Feedback();
    void initialise(int piType);
    void update(int piSection, QString psMessage = "");
    void clear();
private:
    Ui::Feedback *ui;
    int miType;
};

#endif // FEEDBACK_H
