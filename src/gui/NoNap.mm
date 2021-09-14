//*****************************************************************

// Copyright (c) 2020 Aaron Wyatt.
// SPDX-FileCopyrightText: 2021 Aaron Wyatt
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
