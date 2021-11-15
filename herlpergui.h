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
    void updateDisplay(double msTol, int nSlots, int lostWin);
public slots:
    void sliderMoved(double);
    void slider_2Moved(int);
    void slider_3Moved(int);
signals:
    void moved(double);
    void moved_2(int);
    void moved_3(int);

private:
    Ui::HerlperGUI *ui;
};

#endif // HERLPERGUI_H
