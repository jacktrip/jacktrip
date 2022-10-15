//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2022 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
//*****************************************************************

/**
 * \file vsConstants.h
 * \author Nelson Wang
 * \date Oct 2022
 */

#ifndef VSCONSTANTS_H
#define VSCONSTANTS_H

#include <QString>

const QString AUTH_AUTHORIZE_URI = QStringLiteral("https://auth.jacktrip.org/authorize");
const QString AUTH_TOKEN_URI = QStringLiteral("https://auth.jacktrip.org/oauth/token");
const QString AUTH_AUDIENCE  = QStringLiteral("https://api.jacktrip.org");
const QString AUTH_CLIENT_ID = QStringLiteral("cROUJag0UVKDaJ6jRAKRzlVjKVFNU39I");
const QString PROD_API_HOST  = QStringLiteral("app.jacktrip.org");
const QString TEST_API_HOST  = QStringLiteral("test.jacktrip.org");

#endif  // VSCONSTANTS_H
