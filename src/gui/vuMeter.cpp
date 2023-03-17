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

#include "vuMeter.h"

#include <cmath>

VuMeter::VuMeter(QWidget* parent) : QWidget(parent), m_level(0)
{
    m_greenOn.setRgb(97, 197, 84);
    m_greenOff.setRgb(29, 67, 24);
    m_yellowOn.setRgb(245, 191, 79);
    m_yellowOff.setRgb(85, 65, 22);
    m_redOn.setRgb(242, 27, 27);
    m_redOff.setRgb(84, 4, 4);

    m_threshhold1 = std::round(m_bins * 0.6);
    m_threshhold2 = std::round(m_bins * 0.8);
}

void VuMeter::setLevel(qreal level)
{
    m_level = level;
    update();
}

void VuMeter::paintEvent([[maybe_unused]] QPaintEvent* event)
{
    quint32 binWidth = std::floor((width() - ((m_bins - 1) * m_margins)) / m_bins);
    QPainter painter(this);

    painter.setPen(Qt::NoPen);
    quint32 level = std::round(m_level * (m_bins + 1));
    for (quint32 i = 0; i < m_bins; i++) {
        bool on = level > i;
        if (on) {
            if (i < m_threshhold1) {
                painter.setBrush(m_greenOn);
            } else if (i < m_threshhold2) {
                painter.setBrush(m_yellowOn);
            } else {
                painter.setBrush(m_redOn);
            }
        } else {
            if (i < m_threshhold1) {
                painter.setBrush(m_greenOff);
            } else if (i < m_threshhold2) {
                painter.setBrush(m_yellowOff);
            } else {
                painter.setBrush(m_redOff);
            }
        }

        painter.drawRect((binWidth + m_margins) * i, 0, binWidth, height());
    }
}
