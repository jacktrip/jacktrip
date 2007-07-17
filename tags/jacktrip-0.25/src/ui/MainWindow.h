/****************************************************************************
** Form interface generated from reading ui file 'ui/MainWindow.ui'
**
** Created by: The User Interface Compiler ($Id: qt/main.cpp   3.3.7   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qmainwindow.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QFrame;
class QGroupBox;
class QLineEdit;
class QLabel;
class QRadioButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~MainWindow();

    QFrame* frame4;
    QGroupBox* groupBox2_2;
    QLineEdit* dirLineEdit;
    QLabel* fullpathTextLabel;
    QLabel* textLabel1;
    QGroupBox* groupBox2;
    QLabel* sampleRateTextLabel;
    QLabel* netHarpStringsTextLabel;
    QLabel* audioInQInAudioBufsTextLabel;
    QLabel* framesPerAudioBufferTextLabel;
    QLabel* networkPortOffsetTextLabel;
    QLabel* runModeTextLabel;
    QLabel* secondsBetweenPlucksTextLabel;
    QLabel* lowPassFilterCoeffTextLabel;
    QLabel* delayIncBtwnStringsTextLabel;
    QLabel* audioChannelsTextLabel;
    QLabel* runFifoTextLabel;
    QLabel* audioDeviceIDTextLabel;
    QLabel* jackTextLabel;
    QLabel* netInQInPacketsTextLabel;
    QLabel* redundancyTextLabel;
    QFrame* plotFrame;
    QRadioButton* goButton;
    QMenuBar *menubar;
    QPopupMenu *fileMenu;
    QAction* fileNewAction;
    QAction* fileOpenAction;
    QAction* fileSaveAction;
    QAction* fileSaveAsAction;
    QAction* filePrintAction;
    QAction* fileExitAction;

public slots:
    virtual void quitSlot();

protected:

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;
    QPixmap image1;
    QPixmap image2;
    QPixmap image3;

};

#endif // MAINWINDOW_H
