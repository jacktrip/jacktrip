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

#include "NoNap.h"
#include <Foundation/Foundation.h>

NoNap::NoNap() :
    m_preventNap(false)
{}

void NoNap::disableNap()
{
    if (m_preventNap) {
        return;
    }
    m_preventNap = true;
    m_activity = [[NSProcessInfo processInfo] beginActivityWithOptions:NSActivityLatencyCritical | NSActivityUserInitiated reason:@"Disable App Nap"];
    [m_activity retain];
}

void NoNap::enableNap()
{
    if (!m_preventNap) {
        return;
    }
    m_preventNap = false;
    [[NSProcessInfo processInfo] endActivity:m_activity];
    [m_activity release];
}

NoNap::~NoNap()
{
    if (m_preventNap) {
        enableNap();
    }
}
