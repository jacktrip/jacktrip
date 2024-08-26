//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2024 Michael Dickey, Aaron Wyatt.

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
 * \file UserInterface.h
 * \author Michael Dickey
 * \date August 2024
 */

#ifndef __USERINTERFACE_H__
#define __USERINTERFACE_H__

#include <QApplication>
#include <QMainWindow>
#include <QSharedPointer>

#include "Settings.h"

#ifdef __APPLE__
#include "NoNap.h"
#endif

// forward class declarations
class VirtualStudio;
class QJackTrip;

/// UserInterface manages graphical user interfaces for JackTrip
class UserInterface
{
   public:
    /**
     * @brief which GUI mode is in use
     *
     * MODE_UNSET: none selected yet
     * MODE_VS: Virtual Studio mode (QML/Quick interface)
     * MODE_CLASSIC: Classic mode (QJackTrip interface)
     */
    enum uiModeT { MODE_UNSET, MODE_VS, MODE_CLASSIC };

    /// construction requires command line settings
    explicit UserInterface(QSharedPointer<Settings>& settings);

    /// @brief simple destructor
    ~UserInterface();

    /// @return current GUI mode
    inline uiModeT getMode() const { return m_uiMode; }

    /// @return command line settings
    inline Settings& getSettings() { return *m_cliSettings.data(); }

    /// @brief creates new application using command line arguments
    static QCoreApplication* createApplication(int& argc, char* argv[]);

    /// @brief starts graphical user interface
    void start(QApplication* app);

    /// @brief sets GUI mode
    void setMode(uiModeT m);

    /// @brief enables napping for OSX
    void enableNap();

    /// @brief disables napping for OSX
    void disableNap();

   private:
    uiModeT m_uiMode = MODE_UNSET;
    QSharedPointer<Settings> m_cliSettings;

#ifndef NO_VS
    QSharedPointer<VirtualStudio> m_vs_ui;
#endif

#ifndef NO_CLASSIC
    QSharedPointer<QJackTrip> m_classic_ui;
#endif

#ifdef __APPLE__
    NoNap m_noNap;
#endif
};

#endif
