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

#ifndef VUDIALOG_H
#define VUDIALOG_H

#include <QDialog>
#include <QList>
#include <QScopedPointer>
#include "vuMeter.h"

namespace Ui
{
class VuDialog;
}

class VuDialog : public QDialog
{
    Q_OBJECT

   public:
    explicit VuDialog(QWidget* parent = nullptr, quint32 inputChannels = 1, quint32 outputChannels = 1);
    ~VuDialog() override;

    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

   public slots:
    void updatedInputMeasurements(const QVector<float> valuesInDb);
    void updatedOutputMeasurements(const QVector<float> valuesInDb);

   private slots:
    void savePosition();

   private:
    QScopedPointer<Ui::VuDialog> m_ui;
    
    quint32 m_inputChannels;
    quint32 m_outputChannels;
    float m_meterMax = 0.0;
    float m_meterMin = -64.0;
    
    QList<VuMeter*> m_inputMeters;
    QList<VuMeter*> m_outputMeters;
};

#endif  // VUDIALOG_H
