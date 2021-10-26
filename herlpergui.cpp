#include "herlpergui.h"
#include "ui_herlpergui.h"
#include <QDebug>

HerlperGUI::HerlperGUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HerlperGUI)
{

    ui->setupUi(this);
    connect(ui->slider, SIGNAL(valueChanged(int)),
            ui->lcdNumber, SLOT(display(int)));
    connect(ui->slider, SIGNAL(valueChanged(int)),
            this, SLOT(sliderMoved(int)));
    connect(ui->slider_2, SIGNAL(valueChanged(int)),
            ui->lcdNumber_2, SLOT(display(int)));
    connect(ui->slider_2, SIGNAL(valueChanged(int)),
            this, SLOT(slider_2Moved(int)));
    this->show();
}

void HerlperGUI::sliderMoved(int x)
{
    emit moved(x);
}

void HerlperGUI::slider_2Moved(int x)
{
    emit moved_2(x);
}

HerlperGUI::~HerlperGUI()
{
    delete ui;
}
