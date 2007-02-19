/****************************************************************************
** Form implementation generated from reading ui file 'ui/MainWindow.ui'
**
** Created by: The User Interface Compiler ($Id: qt/main.cpp   3.3.7   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "MainWindow.h"

#include <qvariant.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>

static const char* const image0_data[] = { 
"22 22 7 1",
". c None",
"# c #000000",
"b c #292c29",
"c c #5a5d5a",
"d c #838583",
"e c #c5c2c5",
"a c #ffffff",
"......................",
"....##########........",
"....#aaaaaaa#b#.......",
"....#aaaaaaa#cb#......",
"....#aaaaaaa#dcb#.....",
"....#aaaaaaa#edcb#....",
"....#aaaaaaa#aedcb#...",
"....#aaaaaaa#######...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....###############...",
"......................",
"......................"};

static const char* const image1_data[] = { 
"22 22 5 1",
". c None",
"# c #000000",
"c c #838100",
"a c #ffff00",
"b c #ffffff",
"......................",
"......................",
"......................",
"............####....#.",
"...........#....##.##.",
"..................###.",
".................####.",
".####...........#####.",
"#abab##########.......",
"#babababababab#.......",
"#ababababababa#.......",
"#babababababab#.......",
"#ababab###############",
"#babab##cccccccccccc##",
"#abab##cccccccccccc##.",
"#bab##cccccccccccc##..",
"#ab##cccccccccccc##...",
"#b##cccccccccccc##....",
"###cccccccccccc##.....",
"##cccccccccccc##......",
"###############.......",
"......................"};

static const char* const image2_data[] = { 
"22 22 5 1",
". c None",
"# c #000000",
"a c #838100",
"b c #c5c2c5",
"c c #cdb6d5",
"......................",
".####################.",
".#aa#bbbbbbbbbbbb#bb#.",
".#aa#bbbbbbbbbbbb#bb#.",
".#aa#bbbbbbbbbcbb####.",
".#aa#bbbccbbbbbbb#aa#.",
".#aa#bbbccbbbbbbb#aa#.",
".#aa#bbbbbbbbbbbb#aa#.",
".#aa#bbbbbbbbbbbb#aa#.",
".#aa#bbbbbbbbbbbb#aa#.",
".#aa#bbbbbbbbbbbb#aa#.",
".#aaa############aaa#.",
".#aaaaaaaaaaaaaaaaaa#.",
".#aaaaaaaaaaaaaaaaaa#.",
".#aaa#############aa#.",
".#aaa#########bbb#aa#.",
".#aaa#########bbb#aa#.",
".#aaa#########bbb#aa#.",
".#aaa#########bbb#aa#.",
".#aaa#########bbb#aa#.",
"..##################..",
"......................"};

static const char* const image3_data[] = { 
"22 22 88 2",
"Qt c None",
".2 c #000000",
".S c #08ff08",
"#v c #100810",
".U c #101010",
"#c c #101018",
".M c #181018",
"#e c #181818",
".A c #181820",
".L c #201820",
"#l c #202020",
".z c #202029",
"#m c #292029",
"#u c #292829",
"#n c #292831",
".R c #29ff29",
"#o c #312831",
".T c #313031",
"#p c #313039",
".Z c #31ff31",
"#q c #393039",
"#t c #393839",
".y c #393841",
"#s c #413841",
".o c #414041",
"#h c #4a4852",
".n c #5a505a",
"#r c #5a5962",
".I c #5ace5a",
"#b c #6a616a",
".p c #6a696a",
".x c #6a6973",
".Y c #6aff62",
".l c #736973",
".t c #7b717b",
".s c #7b7183",
".0 c #7bff7b",
".r c #837983",
".u c #83798b",
"#g c #83858b",
".v c #8b7994",
"#i c #8b858b",
".w c #8b8594",
"#j c #8b8d8b",
".8 c #8b8d94",
".m c #948d94",
"#k c #948d9c",
"#f c #949594",
".q c #94959c",
".J c #94c694",
"#d c #9c959c",
"#a c #9c95a4",
".k c #9c9d9c",
".N c #9c9da4",
".H c #9ccea4",
".K c #a49da4",
"#. c #a49dac",
".i c #a4a5a4",
".3 c #a4a5ac",
"## c #ac9dac",
".V c #aca5ac",
".d c #acaeac",
".j c #acaeb4",
".9 c #b4aeb4",
".# c #b4b6b4",
".a c #bdbebd",
".7 c #bdd6bd",
".c c #c5c6c5",
".5 c #cdc6cd",
".b c #cdcecd",
".4 c #cdced5",
".F c #d5ced5",
".G c #d5cede",
".h c #d5d6d5",
".E c #d5d6de",
".Q c #d5ffd5",
".B c #ded6de",
".1 c #ded6e6",
".g c #dedede",
".D c #dedee6",
".6 c #e6dee6",
".f c #e6e6e6",
".C c #e6e6ee",
".X c #e6ffe6",
".O c #eee6ee",
".e c #eeeeee",
".W c #f6f6f6",
".P c #ffffff",
"QtQtQtQtQtQt.#.a.b.b.b.b.c.c.a.a.d.aQtQtQtQt",
"QtQtQtQtQtQt.a.e.f.f.f.f.f.e.e.e.g.aQtQtQtQt",
"QtQtQtQtQtQt.a.c.c.c.b.b.c.c.c.c.a.cQtQtQtQt",
"QtQtQtQtQtQt.#.a.a.a.a.#.a.a.#.#.d.aQtQtQtQt",
"QtQtQtQtQt.c.d.c.a.c.c.c.a.a.a.c.#QtQtQtQtQt",
"QtQtQtQtQt.a.a.#.a.a.a.a.a.a.c.c.#QtQtQtQtQt",
"QtQtQtQtQt.a.#.c.a.a.a.a.a.c.a.c.dQtQtQtQtQt",
"QtQtQtQtQt.c.a.a.a.a.a.a.a.a.a.a.#QtQtQtQtQt",
"QtQtQtQtQt.d.b.f.g.g.g.g.g.g.h.g.i.i.jQtQtQt",
"QtQtQt.a.k.l.#.h.b.h.b.h.b.h.g.g.m.n.o.p.#Qt",
"QtQt.a.q.r.s.t.t.t.t.t.t.t.u.v.w.x.y.z.A.o.i",
"Qt.a.k.B.C.D.B.E.E.E.E.F.G.H.I.J.K.o.L.L.M.y",
".a.N.O.P.P.P.P.P.P.P.P.P.Q.R.S.R.b.v.T.A.U.L",
".V.W.P.P.P.P.P.P.P.P.P.P.X.Y.Z.0.P.1.t.A.2.L",
".3.E.4.5.4.h.E.E.g.6.D.B.D.E.7.F.4.5.8.M.2.A",
".m.9.j.V.3#..3.K#.#..i#..K#.###a.q.8#b#c.2.L",
".m.j.j#..3.K.K.K.N.K.N.N.N.N#a#d#d.w#b#c.2#e",
"#f#.#..K.N.K.N.N.N#a.k#a#d#d#d#a.m#g#b.M.2#h",
".m.3.K.K#a.k#a#d#a.k#a#d#a#d.q.m.8#i.x#c#e.d",
"#f#g#i.w#j.w#i.8.w#i.8.8.m.8.m#k.8.w#b#e#fQt",
".#.l.z.A#l.z#m#m#m#n#o#o#p#p#q#q#p#o#p#fQtQt",
"QtQt.d#r#s#s#t#p.T.T.T#u#u.z#e#e#v.o.kQtQtQt"};


/*
 *  Constructs a MainWindow as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
MainWindow::MainWindow( QWidget* parent, const char* name, WFlags fl )
    : QMainWindow( parent, name, fl ),
      image0( (const char **) image0_data ),
      image1( (const char **) image1_data ),
      image2( (const char **) image2_data ),
      image3( (const char **) image3_data )
{
    (void)statusBar();
    if ( !name )
	setName( "MainWindow" );
    setFocusPolicy( QMainWindow::StrongFocus );
    setCentralWidget( new QWidget( this, "qt_central_widget" ) );

    frame4 = new QFrame( centralWidget(), "frame4" );
    frame4->setGeometry( QRect( 20, 10, 410, 120 ) );
    frame4->setFrameShape( QFrame::StyledPanel );
    frame4->setFrameShadow( QFrame::Raised );

    groupBox2_2 = new QGroupBox( frame4, "groupBox2_2" );
    groupBox2_2->setGeometry( QRect( 10, 20, 190, 60 ) );

    dirLineEdit = new QLineEdit( groupBox2_2, "dirLineEdit" );
    dirLineEdit->setGeometry( QRect( 10, 30, 170, 23 ) );
    dirLineEdit->setFocusPolicy( QLineEdit::ClickFocus );

    fullpathTextLabel = new QLabel( frame4, "fullpathTextLabel" );
    fullpathTextLabel->setGeometry( QRect( 140, 90, 250, 20 ) );

    textLabel1 = new QLabel( frame4, "textLabel1" );
    textLabel1->setGeometry( QRect( 220, 10, 160, 94 ) );

    groupBox2 = new QGroupBox( centralWidget(), "groupBox2" );
    groupBox2->setGeometry( QRect( 30, 140, 210, 330 ) );

    sampleRateTextLabel = new QLabel( groupBox2, "sampleRateTextLabel" );
    sampleRateTextLabel->setGeometry( QRect( 20, 21, 160, 20 ) );

    netHarpStringsTextLabel = new QLabel( groupBox2, "netHarpStringsTextLabel" );
    netHarpStringsTextLabel->setGeometry( QRect( 20, 40, 160, 20 ) );

    audioInQInAudioBufsTextLabel = new QLabel( groupBox2, "audioInQInAudioBufsTextLabel" );
    audioInQInAudioBufsTextLabel->setGeometry( QRect( 20, 140, 160, 20 ) );

    framesPerAudioBufferTextLabel = new QLabel( groupBox2, "framesPerAudioBufferTextLabel" );
    framesPerAudioBufferTextLabel->setGeometry( QRect( 20, 160, 160, 20 ) );

    networkPortOffsetTextLabel = new QLabel( groupBox2, "networkPortOffsetTextLabel" );
    networkPortOffsetTextLabel->setGeometry( QRect( 20, 180, 160, 20 ) );

    runModeTextLabel = new QLabel( groupBox2, "runModeTextLabel" );
    runModeTextLabel->setGeometry( QRect( 20, 200, 160, 20 ) );

    secondsBetweenPlucksTextLabel = new QLabel( groupBox2, "secondsBetweenPlucksTextLabel" );
    secondsBetweenPlucksTextLabel->setGeometry( QRect( 20, 220, 160, 20 ) );

    lowPassFilterCoeffTextLabel = new QLabel( groupBox2, "lowPassFilterCoeffTextLabel" );
    lowPassFilterCoeffTextLabel->setGeometry( QRect( 20, 240, 160, 20 ) );

    delayIncBtwnStringsTextLabel = new QLabel( groupBox2, "delayIncBtwnStringsTextLabel" );
    delayIncBtwnStringsTextLabel->setGeometry( QRect( 20, 260, 160, 20 ) );

    audioChannelsTextLabel = new QLabel( groupBox2, "audioChannelsTextLabel" );
    audioChannelsTextLabel->setGeometry( QRect( 20, 60, 160, 20 ) );

    runFifoTextLabel = new QLabel( groupBox2, "runFifoTextLabel" );
    runFifoTextLabel->setGeometry( QRect( 20, 80, 160, 20 ) );

    audioDeviceIDTextLabel = new QLabel( groupBox2, "audioDeviceIDTextLabel" );
    audioDeviceIDTextLabel->setGeometry( QRect( 20, 100, 160, 20 ) );

    jackTextLabel = new QLabel( groupBox2, "jackTextLabel" );
    jackTextLabel->setGeometry( QRect( 20, 280, 160, 20 ) );

    netInQInPacketsTextLabel = new QLabel( groupBox2, "netInQInPacketsTextLabel" );
    netInQInPacketsTextLabel->setGeometry( QRect( 20, 120, 160, 20 ) );

    redundancyTextLabel = new QLabel( groupBox2, "redundancyTextLabel" );
    redundancyTextLabel->setGeometry( QRect( 20, 300, 160, 20 ) );

    plotFrame = new QFrame( centralWidget(), "plotFrame" );
    plotFrame->setGeometry( QRect( 250, 149, 361, 321 ) );
    plotFrame->setFrameShape( QFrame::StyledPanel );
    plotFrame->setFrameShadow( QFrame::Raised );

    goButton = new QRadioButton( centralWidget(), "goButton" );
    goButton->setGeometry( QRect( 450, 39, 104, 21 ) );

    // actions
    fileNewAction = new QAction( this, "fileNewAction" );
    fileNewAction->setIconSet( QIconSet( image0 ) );
    fileOpenAction = new QAction( this, "fileOpenAction" );
    fileOpenAction->setIconSet( QIconSet( image1 ) );
    fileSaveAction = new QAction( this, "fileSaveAction" );
    fileSaveAction->setIconSet( QIconSet( image2 ) );
    fileSaveAsAction = new QAction( this, "fileSaveAsAction" );
    filePrintAction = new QAction( this, "filePrintAction" );
    filePrintAction->setIconSet( QIconSet( image3 ) );
    fileExitAction = new QAction( this, "fileExitAction" );


    // toolbars


    // menubar
    menubar = new QMenuBar( this, "menubar" );


    fileMenu = new QPopupMenu( this );
    fileMenu->insertSeparator();
    fileMenu->insertSeparator();
    fileExitAction->addTo( fileMenu );
    menubar->insertItem( QString(""), fileMenu, 1 );

    languageChange();
    resize( QSize(619, 531).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( fileExitAction, SIGNAL( activated() ), this, SLOT( close() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
MainWindow::~MainWindow()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void MainWindow::languageChange()
{
    setCaption( tr( "Clapper Experiment" ) );
    groupBox2_2->setTitle( tr( "Directory for files" ) );
    dirLineEdit->setText( tr( "/zap/trials" ) );
    fullpathTextLabel->setText( tr( ".wav" ) );
    textLabel1->setText( tr( "Start jack first with\n"
"qjackctl &\n"
"\n"
"and make the buffersize\n"
"and sample rate match\n"
"" ) );
    groupBox2->setTitle( tr( "settings" ) );
    sampleRateTextLabel->setText( tr( "sampleRate" ) );
    netHarpStringsTextLabel->setText( tr( "netHarpStrings" ) );
    audioInQInAudioBufsTextLabel->setText( tr( "audioInQInAudioBufs" ) );
    framesPerAudioBufferTextLabel->setText( tr( "framesPerAudioBuffer" ) );
    networkPortOffsetTextLabel->setText( tr( "networkPortOffset" ) );
    runModeTextLabel->setText( tr( "runMode" ) );
    secondsBetweenPlucksTextLabel->setText( tr( "secondsBetweenPlucks" ) );
    lowPassFilterCoeffTextLabel->setText( tr( "lowpassFilterCoeff" ) );
    delayIncBtwnStringsTextLabel->setText( tr( "delayIncBtwnStrings" ) );
    audioChannelsTextLabel->setText( tr( "audioChannels" ) );
    runFifoTextLabel->setText( tr( "runFifo" ) );
    audioDeviceIDTextLabel->setText( tr( "audioDeviceID" ) );
    jackTextLabel->setText( tr( "jack" ) );
    netInQInPacketsTextLabel->setText( tr( "netInQInPackets" ) );
    redundancyTextLabel->setText( tr( "redundancy" ) );
    goButton->setText( tr( "running" ) );
    fileNewAction->setText( tr( "New" ) );
    fileNewAction->setMenuText( tr( "&New" ) );
    fileNewAction->setAccel( tr( "Ctrl+N" ) );
    fileOpenAction->setText( tr( "Open" ) );
    fileOpenAction->setMenuText( tr( "&Open..." ) );
    fileOpenAction->setAccel( tr( "Ctrl+O" ) );
    fileSaveAction->setText( tr( "Save" ) );
    fileSaveAction->setMenuText( tr( "&Save" ) );
    fileSaveAction->setAccel( tr( "Ctrl+S" ) );
    fileSaveAsAction->setText( tr( "Save As" ) );
    fileSaveAsAction->setMenuText( tr( "Save &As..." ) );
    fileSaveAsAction->setAccel( QString::null );
    filePrintAction->setText( tr( "Print" ) );
    filePrintAction->setMenuText( tr( "&Print..." ) );
    filePrintAction->setAccel( tr( "Ctrl+P" ) );
    fileExitAction->setText( tr( "Exit" ) );
    fileExitAction->setMenuText( tr( "E&xit" ) );
    fileExitAction->setAccel( tr( "Ctrl+Q" ) );
    if (menubar->findItem(1))
        menubar->findItem(1)->setText( tr( "&Session" ) );
}

void MainWindow::quitSlot()
{
    qWarning( "MainWindow::quitSlot(): Not implemented yet" );
}

