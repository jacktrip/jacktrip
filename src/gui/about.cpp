//*****************************************************************
/*
  QJackTrip: Bringing a graphical user interface to JackTrip, a
  system for high quality audio network performance over the
  internet.

  Copyright (c) 2020 Aaron Wyatt.
  
  This file is part of QJackTrip.

  QJackTrip is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  QJackTrip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with QJackTrip.  If not, see <https://www.gnu.org/licenses/>.
*/
//*****************************************************************

#include "about.h"
#include "ui_about.h"
#include "../jacktrip_globals.h"

const QString About::sBuildID = "2021050500";

About::About(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::About)
{
    m_ui->setupUi(this);
    connect(m_ui->closeButton, &QPushButton::clicked, this, [=](){ this->done(0); });
    
    m_ui->aboutLabel->setText(m_ui->aboutLabel->text().replace("%VERSION%", gVersion));
    m_ui->aboutLabel->setText(m_ui->aboutLabel->text().replace("%BUILD%", sBuildID));
#ifdef __MAC_OSX__
    m_ui->aboutImage->setPixmap(QPixmap(":/qjacktrip/about@2x.png"));
#endif
    
    aboutText.setHtml(m_ui->aboutLabel->text());
    aboutText.setDefaultFont(m_ui->aboutLabel->font());
}

void About::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    aboutText.setTextWidth(m_ui->textLayout->geometry().width());
    m_ui->aboutLabel->setMinimumHeight(aboutText.size().height());
}

About::~About() = default;
