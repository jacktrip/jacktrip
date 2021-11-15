#include "herlpergui.h"
#include "ui_herlpergui.h"
#include <QDebug>

HerlperGUI::HerlperGUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HerlperGUI)
{

    ui->setupUi(this);
    connect(ui->slider, SIGNAL(valueChanged(double)),
            ui->lcdNumber, SLOT(display(double)));
    connect(ui->slider, SIGNAL(sliderMoved(double)),
            this, SLOT(sliderMoved(double)));

    connect(ui->slider_2, SIGNAL(valueChanged(int)),
            ui->lcdNumber_2, SLOT(display(int)));
    connect(ui->slider_2, SIGNAL(sliderMoved(int)),
            this, SLOT(slider_2Moved(int)));

    connect(ui->slider_3, SIGNAL(valueChanged(int)),
            ui->lcdNumber_3, SLOT(display(int)));
    connect(ui->slider_3, SIGNAL(sliderMoved(int)),
            this, SLOT(slider_3Moved(int)));
    this->show();
}

void HerlperGUI::updateDisplay(double msTol, int nSlots, int lostWin)
{
    ui->slider->setValue(msTol);
    ui->slider_2->setValue(nSlots);
    ui->slider_3->setValue(lostWin);
    ui->lcdNumber->display(msTol);
    ui->lcdNumber_2->display(nSlots);
    ui->lcdNumber_3->display(lostWin);
}

void HerlperGUI::sliderMoved(double x)
{
    emit moved(x);
}

void HerlperGUI::slider_2Moved(int x)
{
    emit moved_2(x);
}

void HerlperGUI::slider_3Moved(int x)
{
    emit moved_3(x);
}

HerlperGUI::~HerlperGUI()
{
    delete ui;
}
