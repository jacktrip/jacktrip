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

#include "../jacktrip_globals.h"
#include "ui_about.h"

#ifdef BUILD_TYPE
const QString About::s_buildType = QStringLiteral(BUILD_TYPE);
#else
const QString About::s_buildType = QLatin1String("");
#endif
#ifdef BUILD_ID
const QString About::s_buildID = QStringLiteral(BUILD_ID);
#elif defined(JACKTRIP_BUILD_INFO)
#define STR(s)       #s
#define TO_STRING(s) STR(s)
const QString About::s_buildID   = QLatin1String(TO_STRING(JACKTRIP_BUILD_INFO));
#else
const QString About::s_buildID = QLatin1String("");
#endif

About::About(QWidget* parent) : QDialog(parent), m_ui(new Ui::About)
{
    m_ui->setupUi(this);
    connect(m_ui->closeButton, &QPushButton::clicked, this, [this]() {
        this->done(0);
    });

    // Replace %VERSION% and %QTVERSION%
    m_ui->aboutLabel->setText(
        m_ui->aboutLabel->text().replace(QLatin1String("%VERSION%"), gVersion));
    m_ui->aboutLabel->setText(
        m_ui->aboutLabel->text().replace(QLatin1String("%QTVERSION%"), qVersion()));

    // Replace %LICENSE%
    QString licenseText;
#if defined(_WIN32) && defined(RT_AUDIO)
    licenseText = QLatin1String(
        "This build of JackTrip includes support for ASIO. ASIO is a trademark and "
        "software of Steinberg Media Technologies GmbH.</p><p></p><p>");
#endif
#ifdef QT_OPENSOURCE
    licenseText += QLatin1String("This build of JackTrip is subject to LGPL license. ");
#endif
    m_ui->aboutLabel->setText(
        m_ui->aboutLabel->text().replace(QLatin1String("%LICENSE%"), licenseText));

    // Replace %BUILD%
    QString buildString;
    if (!s_buildType.isEmpty() || !s_buildID.isEmpty()) {
        buildString = QStringLiteral("<br/>(");
        if (!s_buildType.isEmpty()) {
            buildString.append(s_buildType);
            if (!s_buildID.isEmpty()) {
                buildString.append(QStringLiteral(", build %1").arg(s_buildID));
            }
        } else {
            buildString.append(QStringLiteral("Build %1").arg(s_buildID));
        }
        buildString.append(")");
    }
    m_ui->aboutLabel->setText(
        m_ui->aboutLabel->text().replace(QLatin1String("%BUILD%"), buildString));

#ifdef __APPLE__
    m_ui->aboutImage->setPixmap(QPixmap(":/images/icon_128@2x.png"));
#endif

    aboutText.setHtml(m_ui->aboutLabel->text());
    aboutText.setDefaultFont(m_ui->aboutLabel->font());
}

void About::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    aboutText.setTextWidth(m_ui->textLayout->geometry().width());
    m_ui->aboutLabel->setMinimumHeight(aboutText.size().height());
}

About::~About() = default;
