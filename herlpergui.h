#ifndef HERLPERGUI_H
#define HERLPERGUI_H

#include <QDialog>

namespace Ui {
class HerlperGUI;
}

class HerlperGUI : public QDialog
{
    Q_OBJECT

public:
    explicit HerlperGUI(QWidget *parent = nullptr);
    ~HerlperGUI();
public slots:
    void sliderMoved(int);
    void slider_2Moved(int);
signals:
    void moved(int);
    void moved_2(int);

private:
    Ui::HerlperGUI *ui;
};

#endif // HERLPERGUI_H
