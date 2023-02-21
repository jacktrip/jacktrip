//*****************************************************************
/*
  QJackTrip: Bringing a graphical user interface to JackTrip, a
  system for high quality audio network performance over the
  internet.

  Copyright (c) 2022 Aaron Wyatt.

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

#ifndef VUMETER_H
#define VUMETER_H

#include <QPainter>
#include <QWidget>

class VuMeter : public QWidget
{
    Q_OBJECT

   public:
    VuMeter(QWidget* parent = nullptr);
    ~VuMeter() override = default;

    void setLevel(qreal level);

   protected:
    void paintEvent(QPaintEvent* event) override;

   private:
    qreal m_level;
    QColor m_greenOn;
    QColor m_greenOff;
    QColor m_yellowOn;
    QColor m_yellowOff;
    QColor m_redOn;
    QColor m_redOff;

    quint32 m_bins    = 30;
    quint32 m_margins = 2;
    quint32 m_threshhold1;
    quint32 m_threshhold2;
};

#endif  // VUMETER_H
