//*****************************************************************

// Copyright (c) 2020 Aaron Wyatt.
// SPDX-FileCopyrightText: 2021 Aaron Wyatt
//
// SPDX-License-Identifier: GPL-3.0-or-later

//*****************************************************************

#ifndef __NONAP_H__
#define __NONAP_H__

#include <objc/objc.h>

class NoNap
{
   public:
    NoNap();
    ~NoNap();

    void disableNap();
    void enableNap();

   private:
    id m_activity;
    bool m_preventNap;
};

#endif  // __NONAP_H__
