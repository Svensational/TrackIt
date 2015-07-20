#include "mainwindow.h"
#include <QtGui>
#include "videowidget.h"
#include "datawidget.h"
#include "scrollarea.h"
#include "julia.h"

/**
  * @sa void initGUI()
  */
MainWindow::MainWindow(QWidget * parent) :
   QMainWindow(parent)
{
   initGUI();

   julia = new Julia();
   juliaShortcut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::ALT + Qt::Key_J), this);
   connect(juliaShortcut, SIGNAL(activated()), julia, SLOT(exec()));
}

MainWindow::~MainWindow() {
   delete julia;
}

/** This only concerns actions that need one or more category present.
  */
void MainWindow::categoryCountChanged(int count) {
   newObjAct->setEnabled(count>0);
   editCatAct->setEnabled(count>0);
   deleteCatAct->setEnabled(count>0);
}

void MainWindow::changeMaxFrames(int n) {
   slider->setRange(0, n-1);
}

void MainWindow::createAboutDialog() {
   aboutDialog = new QDialog(this);
   QHBoxLayout * mainLayout = new QHBoxLayout();
   QLabel * iconLabel = new QLabel();
   iconLabel->setPixmap(QPixmap(":/icons/visuslogo_small"));
   mainLayout->addWidget(iconLabel);
   mainLayout->addSpacing(20);
   QVBoxLayout * contentLayout = new QVBoxLayout();
   QLabel * titleLabel = new QLabel("TrackIt");
   QFont font = titleLabel->font();
   font.setPointSize(font.pointSize()*2);
   font.setWeight(QFont::Bold);
   titleLabel->setFont(font);
   contentLayout->addWidget(titleLabel);
   contentLayout->addWidget(new QLabel("Video Ground Truth Editor"));
   contentLayout->addWidget(new QLabel("v1.0"));
   contentLayout->addSpacing(20);
   contentLayout->addWidget(new QLabel("<b>Sven Klingel</b>"));
   contentLayout->addWidget(new QLabel("klingesn@studi.informatik.uni-stuttgart.de"));
   contentLayout->addSpacing(5);
   contentLayout->addWidget(new QLabel("<b>Hanspeter H&auml;gele</b>"));
   contentLayout->addWidget(new QLabel("haegelhr@studi.informatik.uni-stuttgart.de"));
   contentLayout->addSpacing(20);
   contentLayout->addWidget(new QLabel("<i>Softwarepraktikum 2012/2013</i>"));
   contentLayout->addWidget(new QLabel("Visualisation Research Center"));
   contentLayout->addWidget(new QLabel("University of Stuttgart"));
   contentLayout->addWidget(new QLabel("www.vis.uni-stuttgart.de"));
   contentLayout->addSpacing(5);
   contentLayout->addWidget(new QLabel("Supervisor: Dipl. Inf. Kuno Kurzhals"));
   contentLayout->addWidget(new QLabel("kuno.kurzhals@vis.uni-stuttgart.de"));
   contentLayout->addSpacing(20);
   QPushButton * okButton = new QPushButton("&Ok");
   connect(okButton, SIGNAL(clicked()), aboutDialog, SLOT(accept()));
   contentLayout->addWidget(okButton);
   for (int i=0; i<contentLayout->count(); ++i) {
      contentLayout->itemAt(i)->setAlignment(Qt::AlignCenter);
   }
   mainLayout->addLayout(contentLayout);
   mainLayout->addSpacing(20);
   iconLabel = new QLabel();
   iconLabel->setPixmap(QPixmap(":/icons/unilogo_small"));
   mainLayout->addWidget(iconLabel);
   mainLayout->setSizeConstraint(QLayout::SetFixedSize);
   aboutDialog->setLayout(mainLayout);
}

void MainWindow::createActions() {
   QList<QKeySequence> shortcuts;

   // file actions

   QIcon openDataIcon(":/icons/opendata-16");
   openDataIcon.addFile(":/icons/opendata-24");
   openDataAct = new QAction(openDataIcon, tr("Open data"), this);
   openDataAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
   connect(openDataAct, SIGNAL(triggered()), dataWidget, SLOT(openFile()));

   QIcon openVideoIcon(":/icons/openvideo-16");
   openVideoIcon.addFile(":/icons/openvideo-24");
   openVideoAct = new QAction(openVideoIcon, tr("Open video"), this);
   openVideoAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_O));
   connect(openVideoAct, SIGNAL(triggered()), videoWidget, SLOT(openVideoFile()));

   QIcon saveDataIcon(":/icons/save-16");
   saveDataIcon.addFile(":/icons/save-24");
   saveDataAct = new QAction(saveDataIcon, tr("Save data"), this);
   saveDataAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
   connect(saveDataAct, SIGNAL(triggered()), dataWidget, SLOT(saveFile()));

   QIcon saveDataAsIcon(":/icons/saveas-16");
   saveDataAsIcon.addFile(":/icons/saveas-24");
   saveDataAsAct = new QAction(saveDataAsIcon, tr("Save data as..."), this);
   saveDataAsAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));
   connect(saveDataAsAct, SIGNAL(triggered()), dataWidget, SLOT(saveFileAs()));

   QIcon importDataIcon(":/icons/importdata-16");
   importDataIcon.addFile(":/icons/importdata-24");
   importDataAct = new QAction(importDataIcon, tr("Import data"), this);
   importDataAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
   connect(importDataAct, SIGNAL(triggered()), dataWidget, SLOT(importFile()));

   QIcon exportDataIcon(":/icons/exportdata-16");
   exportDataIcon.addFile(":/icons/exportdata-24");
   exportDataAct = new QAction(exportDataIcon, tr("Export data"), this);
   exportDataAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
   connect(exportDataAct, SIGNAL(triggered()), dataWidget, SLOT(exportFile()));

   QIcon quitIcon(":/icons/quit-16");
   //quitIcon.addFile(":/icons/quit-24");
   quitAct = new QAction(quitIcon, tr("&Quit"), this);
   quitAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
   connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

   // navigate actions

   QIcon playPauseIcon(":/icons/play-32");
   playPauseIcon.addFile(":/icons/pause-32", QSize(), QIcon::Normal, QIcon::On);
   playPauseIcon.addFile(":/icons/play-16");
   playPauseIcon.addFile(":/icons/pause-16", QSize(), QIcon::Normal, QIcon::On);
   playPauseAct = new QAction(playPauseIcon, tr("Play/Pause"), this);
   playPauseAct->setCheckable(true);
   shortcuts.clear();
   shortcuts << QKeySequence(Qt::Key_Space) << QKeySequence(Qt::Key_MediaTogglePlayPause);
   playPauseAct->setShortcuts(shortcuts);
   connect(playPauseAct, SIGNAL(triggered(bool)), videoWidget, SLOT(play(bool)));
   connect(videoWidget, SIGNAL(playToggled(bool)), playPauseAct, SLOT(setChecked (bool)));

   QIcon nextFrameIcon(":/icons/next-32");
   nextFrameIcon.addFile(":/icons/next-16");
   nextFrameAct = new QAction(nextFrameIcon, tr("Next frame"), this);
   shortcuts.clear();
   shortcuts << QKeySequence(Qt::Key_Right) << QKeySequence(Qt::Key_D)
             << QKeySequence(Qt::Key_L) << QKeySequence(Qt::Key_MediaNext);
   nextFrameAct->setShortcuts(shortcuts);
   connect(nextFrameAct, SIGNAL(triggered()), videoWidget, SLOT(showNextFrame()));

   QIcon previousFrameIcon(":/icons/previous-32");
   previousFrameIcon.addFile(":/icons/previous-16");
   previousFrameAct = new QAction(previousFrameIcon, tr("Previous frame"), this);
   shortcuts.clear();
   shortcuts << QKeySequence(Qt::Key_Left) << QKeySequence(Qt::Key_A)
             << QKeySequence(Qt::Key_J) << QKeySequence(Qt::Key_MediaPrevious);
   previousFrameAct->setShortcuts(shortcuts);
   connect(previousFrameAct, SIGNAL(triggered()), videoWidget, SLOT(showPreviousFrame()));

   QIcon nextFrameKeyIcon(":/icons/nextkey-32");
   nextFrameKeyIcon.addFile(":/icons/nextkey-16");
   nextFrameKeyAct = new QAction(nextFrameKeyIcon, tr("Next frame w/ keybox"), this);
   shortcuts.clear();
   shortcuts << QKeySequence(Qt::CTRL+Qt::Key_Right) << QKeySequence(Qt::CTRL+Qt::Key_D)
             << QKeySequence(Qt::CTRL+Qt::Key_L) << QKeySequence(Qt::CTRL+Qt::Key_MediaNext);
   nextFrameKeyAct->setShortcuts(shortcuts);
   connect(nextFrameKeyAct, SIGNAL(triggered()), dataWidget, SLOT(selectNextKeyframe()));

   QIcon previousFrameKeyIcon(":/icons/previouskey-32");
   previousFrameKeyIcon.addFile(":/icons/previouskey-16");
   previousFrameKeyAct = new QAction(previousFrameKeyIcon, tr("Previous frame w/ keybox"), this);
   shortcuts.clear();
   shortcuts << QKeySequence(Qt::CTRL+Qt::Key_Left) << QKeySequence(Qt::CTRL+Qt::Key_A)
             << QKeySequence(Qt::CTRL+Qt::Key_J) << QKeySequence(Qt::CTRL+Qt::Key_MediaPrevious);
   previousFrameKeyAct->setShortcuts(shortcuts);
   connect(previousFrameKeyAct, SIGNAL(triggered()), dataWidget, SLOT(selectPreviousKeyframe()));

   nextCategoryAct = new QAction(tr("Next category"), this);
   shortcuts.clear();
   shortcuts << QKeySequence(Qt::Key_PageDown) << QKeySequence(Qt::Key_E)
             << QKeySequence(Qt::Key_O);
   nextCategoryAct->setShortcuts(shortcuts);
   connect(nextCategoryAct, SIGNAL(triggered()), dataWidget, SLOT(selectNextCategory()));

   previousCategoryAct = new QAction(tr("Previous category"), this);
   shortcuts.clear();
   shortcuts << QKeySequence(Qt::Key_PageUp) << QKeySequence(Qt::Key_Q)
             << QKeySequence(Qt::Key_U);
   previousCategoryAct->setShortcuts(shortcuts);
   connect(previousCategoryAct, SIGNAL(triggered()), dataWidget, SLOT(selectPreviousCategory()));

   nextObjectAct = new QAction(tr("Next object"), this);
   shortcuts.clear();
   shortcuts << QKeySequence(Qt::Key_Down) << QKeySequence(Qt::Key_S)
             << QKeySequence(Qt::Key_K);
   nextObjectAct->setShortcuts(shortcuts);
   connect(nextObjectAct, SIGNAL(triggered()), dataWidget, SLOT(selectNextObject()));

   previousObjectAct = new QAction(tr("Previous object"), this);
   shortcuts.clear();
   shortcuts << QKeySequence(Qt::Key_Up) << QKeySequence(Qt::Key_W)
             << QKeySequence(Qt::Key_I);
   previousObjectAct->setShortcuts(shortcuts);
   connect(previousObjectAct, SIGNAL(triggered()), dataWidget, SLOT(selectPreviousObject()));

   // zoom actions

   QIcon zoomInIcon(inactivePixmap(":/icons/zoomin-12"));
   zoomInIcon.addPixmap(QPixmap(":/icons/zoomin-12"), QIcon::Active);
   zoomInAct = new QAction(zoomInIcon, tr("Zoom in"), this);
   zoomInAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Plus));
   connect(zoomInAct, SIGNAL(triggered()), videoWidget, SLOT(zoomIn()));

   QIcon zoomOutIcon(inactivePixmap(":/icons/zoomout-12"));
   zoomOutIcon.addPixmap(QPixmap(":/icons/zoomout-12"), QIcon::Active);
   zoomOutAct = new QAction(zoomOutIcon, tr("Zoom out"), this);
   zoomOutAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus));
   connect(zoomOutAct, SIGNAL(triggered()), videoWidget, SLOT(zoomOut()));

   QIcon zoomResetIcon(inactivePixmap(":/icons/zoom0-12"));
   zoomResetIcon.addPixmap(QPixmap(":/icons/zoom0-12"), QIcon::Active);
   zoomResetAct = new QAction(zoomResetIcon, tr("Reset zoom"), this);
   zoomResetAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_0));
   connect(zoomResetAct, SIGNAL(triggered()), videoWidget, SLOT(zoomReset()));

   QIcon zoomFitIcon(inactivePixmap(":/icons/zoomfit-12"));
   zoomFitIcon.addPixmap(QPixmap(":/icons/zoomfit-12"), QIcon::Active);
   zoomFitIcon.addPixmap(QPixmap(":/icons/zoomfit-12"), QIcon::Normal, QIcon::On);
   zoomFitAct = new QAction(zoomFitIcon, tr("Auto zoom"), this);
   zoomFitAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Comma));
   zoomFitAct->setCheckable(true);
   zoomFitAct->setChecked(true);
   connect(zoomFitAct, SIGNAL(toggled(bool)), videoWidget, SLOT(zoomFit(bool)));
   connect(videoWidget, SIGNAL(zoomChangedManually(bool)), zoomFitAct, SLOT(setChecked(bool)));

   // data actions

   QIcon newBoxIcon(":/icons/newsinglebox-32");
   newBoxIcon.addFile(":/icons/newsinglebox-16");
   newBoxAct = new QAction(newBoxIcon, tr("New bounding box"), this);
   shortcuts.clear();
   shortcuts << QKeySequence(Qt::Key_Insert) << QKeySequence(Qt::Key_R)
             << QKeySequence(Qt::Key_Z);
   newBoxAct->setShortcuts(shortcuts);
   newBoxAct->setCheckable(true);
   newBoxAct->setDisabled(true);
   connect(newBoxAct, SIGNAL(triggered()), videoWidget, SLOT(createBBox()));
   connect(videoWidget, SIGNAL(boxCreationStarted(bool)), newBoxAct, SLOT(setChecked(bool)));

   QIcon newKeyboxIcon(":/icons/newkeybox-32");
   newKeyboxIcon.addFile(":/icons/newkeybox-16");
   newKeyboxAct = new QAction(newKeyboxIcon, tr("New bounding box (key)"), this);
   shortcuts.clear();
   shortcuts << QKeySequence(Qt::SHIFT+Qt::Key_Insert) << QKeySequence(Qt::Key_F)
             << QKeySequence(Qt::Key_H);
   newKeyboxAct->setShortcuts(shortcuts);
   newKeyboxAct->setCheckable(true);
   newKeyboxAct->setDisabled(true);
   connect(newKeyboxAct, SIGNAL(triggered()), videoWidget, SLOT(createKeyBBox()));
   connect(videoWidget, SIGNAL(boxCreationStarted(bool)), newKeyboxAct, SLOT(setChecked(bool)));

   QIcon deleteBoxIcon(":/icons/deletebox-32");
   deleteBoxIcon.addFile(":/icons/deletebox-16");
   deleteBoxAct = new QAction(deleteBoxIcon, tr("Delete bounding box"), this);
   deleteBoxAct->setShortcut(QKeySequence(Qt::Key_Delete));
   //deleteBoxAct->setDisabled(true);
   connect(deleteBoxAct, SIGNAL(triggered()), dataWidget, SLOT(deleteBBox()));

   QIcon newObjIcon(":/icons/newobject-32");
   newObjIcon.addFile(":/icons/newobject-16");
   newObjAct = new QAction(newObjIcon, tr("New object"), this);
   shortcuts.clear();
   shortcuts << QKeySequence(Qt::CTRL+Qt::Key_R);
   newObjAct->setShortcuts(shortcuts);
   connect(newObjAct, SIGNAL(triggered()), dataWidget, SLOT(newObject()));

   QIcon deleteObjIcon(":/icons/deleteobject-32");
   deleteObjIcon.addFile(":/icons/deleteobject-16");
   deleteObjAct = new QAction(deleteObjIcon, tr("Delete object"), this);
   deleteObjAct->setDisabled(true);
   connect(deleteObjAct, SIGNAL(triggered()), dataWidget, SLOT(deleteObject()));

   QIcon editObjIcon(":/icons/editobject-32");
   editObjIcon.addFile(":/icons/editobject-16");
   editObjAct = new QAction(editObjIcon, tr("Move object"), this);
   editObjAct->setDisabled(true);
   connect(editObjAct, SIGNAL(triggered()), dataWidget, SLOT(editObject()));

   QIcon sortByIDIcon(":/icons/sortid-16");
   sortByIDAct = new QAction(sortByIDIcon, tr("Sort objects by ID"), this);
   connect(sortByIDAct, SIGNAL(triggered()), dataWidget, SLOT(sortByID()));

   QIcon sortByFNIcon(":/icons/sortfn-16");
   sortByFNAct = new QAction(sortByFNIcon, tr("Sort objects by frame #"), this);
   connect(sortByFNAct, SIGNAL(triggered()), dataWidget, SLOT(sortByFN()));

   QIcon newCatIcon(":/icons/newcategory-32");
   newCatIcon.addFile(":/icons/newcategory-16");
   newCatAct = new QAction(newCatIcon, tr("New category"), this);
   connect(newCatAct, SIGNAL(triggered()), dataWidget, SLOT(newCategory()));

   QIcon deleteCatIcon(":/icons/deletecategory-32");
   deleteCatIcon.addFile(":/icons/deletecategory-16");
   deleteCatAct = new QAction(deleteCatIcon, tr("Delete category"), this);
   connect(deleteCatAct, SIGNAL(triggered()), dataWidget, SLOT(deleteCategory()));

   QIcon editCatIcon(":/icons/editcategory-32");
   editCatIcon.addFile(":/icons/editcategory-16");
   editCatAct = new QAction(editCatIcon, tr("Edit category"), this);
   connect(editCatAct, SIGNAL(triggered()), dataWidget, SLOT(editCategory()));

   QIcon clearIcon(":/icons/clear-16");
   clearDataAct = new QAction(clearIcon, tr("Clear data"), this);
   connect(clearDataAct, SIGNAL(triggered()), dataWidget, SLOT(clearData()));

   toggleCenterlineAct = new QAction(tr("Show Centerlines"), this);
   toggleCenterlineAct->setShortcut(QKeySequence(Qt::Key_C));
   toggleCenterlineAct->setCheckable(true);
   toggleCenterlineAct->setChecked(true);
   connect(toggleCenterlineAct, SIGNAL(triggered()), videoWidget, SLOT(toggleCenterlines()));

   toggleCacheAct = new QAction(tr("Switch cache usage on/off"), this);
   toggleCacheAct->setCheckable(true);
   toggleCacheAct->setChecked(true);
   connect(toggleCacheAct, SIGNAL(triggered()), videoWidget, SLOT(toggleCache()));

   setCacheProperties = new QAction(tr("Set Cache Properties"), this);
   connect(setCacheProperties, SIGNAL(triggered()), videoWidget, SLOT(setCacheProperties()));

   aboutAction = new QAction(tr("About TrackIt"), this);
   connect(aboutAction, SIGNAL(triggered()), aboutDialog, SLOT(exec()));
}

void MainWindow::createCentralWidget() {
   QWidget * mainWidget = new QWidget();

   QVBoxLayout * videoLayout = new QVBoxLayout();

   ScrollArea * scrollArea = new ScrollArea();
   scrollArea->setAlignment(Qt::AlignCenter);
   scrollArea->setBackgroundRole(QPalette::Dark);
   scrollArea->setWidget(videoWidget);
   connect(scrollArea, SIGNAL(sizeChanged(QSize)), videoWidget, SLOT(setAvailableSize(QSize)));
   videoLayout->addWidget(scrollArea);

   slider = new QSlider(Qt::Horizontal);
   slider->setRange(0, 0);
   slider->setTracking(false);
   videoLayout->addWidget(slider);

   QHBoxLayout * videoControlsLayout = new QHBoxLayout();

   QVBoxLayout * seekLabelLayout = new QVBoxLayout();
   seekLabel = new QLabel();
   seekLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   //QPalette palette;
   //palette.setColor(QPalette::WindowText, palette.color(QPalette::Shadow));
   //seekLabel->setPalette(palette);
   seekLabelLayout->addWidget(seekLabel);

   timeLabel = new QLabel();
   QPalette palette;
   palette.setColor(QPalette::WindowText, palette.color(QPalette::Shadow));
   timeLabel->setPalette(palette);
   seekLabelLayout->addWidget(timeLabel);

   videoControlsLayout->addLayout(seekLabelLayout);
   //videoControlsLayout->setAlignment(seekLabel, Qt::AlignTop);

   QToolButton * previousFrameKeyBtn = new QToolButton();
   previousFrameKeyBtn->setAutoRepeat(true);
   previousFrameKeyBtn->setAutoRaise(true);
   previousFrameKeyBtn->setIconSize(QSize(32, 32));
   previousFrameKeyBtn->setDefaultAction(previousFrameKeyAct);
   videoControlsLayout->addWidget(previousFrameKeyBtn);

   QToolButton * previousFrameBtn = new QToolButton();
   previousFrameBtn->setAutoRepeat(true);
   previousFrameBtn->setAutoRepeatInterval(17);
   previousFrameBtn->setAutoRaise(true);
   previousFrameBtn->setIconSize(QSize(32, 32));
   previousFrameBtn->setDefaultAction(previousFrameAct);
   videoControlsLayout->addWidget(previousFrameBtn);

   QToolButton * playBtn = new QToolButton();
   playBtn->setAutoRaise(true);
   playBtn->setIconSize(QSize(32, 32));
   playBtn->setDefaultAction(playPauseAct);
   videoControlsLayout->addWidget(playBtn);

   QToolButton * nextFrameBtn = new QToolButton();
   nextFrameBtn->setAutoRepeat(true);
   nextFrameBtn->setAutoRepeatInterval(17);
   nextFrameBtn->setAutoRaise(true);
   nextFrameBtn->setIconSize(QSize(32, 32));
   nextFrameBtn->setDefaultAction(nextFrameAct);
   videoControlsLayout->addWidget(nextFrameBtn);

   QToolButton * nextFrameKeyBtn = new QToolButton();
   nextFrameKeyBtn->setAutoRepeat(true);
   nextFrameKeyBtn->setAutoRaise(true);
   nextFrameKeyBtn->setIconSize(QSize(32, 32));
   nextFrameKeyBtn->setDefaultAction(nextFrameKeyAct);
   videoControlsLayout->addWidget(nextFrameKeyBtn);

   videoControlsLayout->addLayout(createZoomLayout());
   videoLayout->addLayout(videoControlsLayout);

   mainWidget->setLayout(videoLayout);
   videoLayout->setContentsMargins(0, 0, 0, 8);
   setCentralWidget(mainWidget);
}

/**
  * Apparently there is only the data dock widget.
  */
void MainWindow::createDockWidgets() {
   QDockWidget * dataDockWidget = new QDockWidget(tr("Data"), this);
   dataDockWidget->setWidget(dataWidget);
   addDockWidget(Qt::BottomDockWidgetArea, dataDockWidget);
}

/** These icons are used in different places so they are defined classwide.
  */
void MainWindow::createIcons() {
   newSingleBoxIcon = QIcon(":/icons/newsinglebox-32");
   newSingleBoxIcon.addFile(":/icons/newsinglebox-16");

   newKeyBoxIcon = QIcon(":/icons/newkeybox-32");
   newKeyBoxIcon.addFile(":/icons/newkeybox-16");

   convertSingleBoxIcon = QIcon(":/icons/convertsinglebox-32");
   convertSingleBoxIcon.addFile(":/icons/convertsinglebox-16");

   convertKeyBoxIcon = QIcon(":/icons/convertkeybox-32");
   convertKeyBoxIcon.addFile(":/icons/convertkeybox-16");
}

void MainWindow::createMenus() {
   QMenu * fileMenu = menuBar()->addMenu(tr("&File"));
   fileMenu->addAction(openDataAct);
   fileMenu->addAction(openVideoAct);
   fileMenu->addSeparator();
   fileMenu->addAction(saveDataAct);
   fileMenu->addAction(saveDataAsAct);
   fileMenu->addSeparator();
   fileMenu->addAction(importDataAct);
   fileMenu->addAction(exportDataAct);
   fileMenu->addSeparator();
   fileMenu->addAction(quitAct);

   QMenu * dataMenu = menuBar()->addMenu(tr("&Data"));
   dataMenu->addAction(newBoxAct);
   dataMenu->addAction(newKeyboxAct);
   dataMenu->addAction(deleteBoxAct);
   dataMenu->addSeparator();
   dataMenu->addAction(newObjAct);
   dataMenu->addAction(deleteObjAct);
   dataMenu->addAction(editObjAct);
   dataMenu->addAction(sortByIDAct);
   dataMenu->addAction(sortByFNAct);
   dataMenu->addSeparator();
   dataMenu->addAction(newCatAct);
   dataMenu->addAction(deleteCatAct);
   dataMenu->addAction(editCatAct);
   dataMenu->addSeparator();
   dataMenu->addAction(clearDataAct);

   QMenu * navMenu = menuBar()->addMenu(tr("&Navigation"));
   navMenu->addAction(playPauseAct);
   navMenu->addAction(nextFrameAct);
   navMenu->addAction(previousFrameAct);
   navMenu->addAction(nextFrameKeyAct);
   navMenu->addAction(previousFrameKeyAct);
   navMenu->addSeparator();
   navMenu->addAction(nextObjectAct);
   navMenu->addAction(previousObjectAct);
   navMenu->addSeparator();
   navMenu->addAction(nextCategoryAct);
   navMenu->addAction(previousCategoryAct);

   QMenu * settingsMenu = menuBar()->addMenu(tr("&Settings"));
   settingsMenu->addAction(toggleCenterlineAct);
   settingsMenu->addAction(toggleCacheAct);
   settingsMenu->addAction(setCacheProperties);

   QMenu * helpMenu = menuBar()->addMenu(tr("?"));
   helpMenu->addAction(aboutAction);
}

/** Actually the possible status bar isn't used, but if you plan to do so here is
  * the place to set it up.
  */
void MainWindow::createStatusBar() {
}

void MainWindow::createToolbars() {
   QToolBar * fileToolBar = new QToolBar(tr("File toolbar"));
   fileToolBar->setIconSize(QSize(24, 24));
   fileToolBar->addAction(openDataAct);
   fileToolBar->addAction(openVideoAct);
   fileToolBar->addSeparator();
   fileToolBar->addAction(saveDataAct);
   fileToolBar->addAction(saveDataAsAct);
   fileToolBar->addSeparator();
   fileToolBar->addAction(importDataAct);
   fileToolBar->addAction(exportDataAct);
   addToolBar(Qt::TopToolBarArea, fileToolBar);


   QToolBar * dataToolBar = new QToolBar(tr("Data toolbar"));
   dataToolBar->setIconSize(QSize(32, 32));
   dataToolBar->addAction(newBoxAct);
   dataToolBar->addAction(newKeyboxAct);
   dataToolBar->addAction(deleteBoxAct);
   dataToolBar->addSeparator();
   dataToolBar->addAction(newObjAct);
   dataToolBar->addAction(deleteObjAct);
   dataToolBar->addAction(editObjAct);
   dataToolBar->addSeparator();
   dataToolBar->addAction(newCatAct);
   dataToolBar->addAction(deleteCatAct);
   dataToolBar->addAction(editCatAct);
   addToolBar(Qt::LeftToolBarArea, dataToolBar);
}

/** This layout is part of the central widget
  */
QLayout * MainWindow::createZoomLayout() {
   QGridLayout * zoomLayout = new QGridLayout();

   zoomLabel = new QLabel();
   zoomLabel->setAlignment(Qt::AlignRight);
   QPalette palette;
   palette.setColor(QPalette::WindowText, palette.color(QPalette::Shadow));
   zoomLabel->setPalette(palette);
   zoomLayout->addWidget(zoomLabel, 0, 1, 1, 4);

   QToolButton * zoomInBtn = new QToolButton();
   zoomInBtn->setAutoRepeat(true);
   zoomInBtn->setAutoRepeatInterval(100);
   zoomInBtn->setAutoRaise(true);
   zoomInBtn->setIconSize(QSize(12, 12));
   zoomInBtn->setDefaultAction(zoomInAct);
   zoomLayout->addWidget(zoomInBtn, 1, 1);

   QToolButton * zoomOutBtn = new QToolButton();
   zoomOutBtn->setAutoRepeat(true);
   zoomOutBtn->setAutoRepeatInterval(100);
   zoomOutBtn->setAutoRaise(true);
   zoomOutBtn->setIconSize(QSize(12, 12));
   zoomOutBtn->setDefaultAction(zoomOutAct);
   zoomLayout->addWidget(zoomOutBtn, 1, 2);

   QToolButton * zoomFitBtn = new QToolButton();
   zoomFitBtn->setAutoRaise(true);
   zoomFitBtn->setIconSize(QSize(12, 12));
   zoomFitBtn->setDefaultAction(zoomFitAct);
   zoomLayout->addWidget(zoomFitBtn, 1, 3);

   QToolButton * zoomFitReset = new QToolButton();
   zoomFitReset->setAutoRaise(true);
   zoomFitReset->setIconSize(QSize(12, 12));
   zoomFitReset->setDefaultAction(zoomResetAct);
   zoomLayout->addWidget(zoomFitReset, 1, 4);

   zoomLayout->setColumnStretch(0, 1);
   zoomLayout->setSpacing(0);

   return zoomLayout;
}

/** The context menu contains all the data actions but all inactive (because
  * currently useless) actions are removed.
  */
void MainWindow::dataContextMenu(QPoint const & pos) {
   QTableView * tableView = static_cast<QTableView *>(dataWidget->currentWidget());
   QHeaderView * headerView = tableView->verticalHeader();

   if (!tableView->underMouse()) {
      return;
   }

   if (headerView->underMouse()) {
      dataWidget->setSelectedObjectByRow(headerView->logicalIndexAt(pos)-1);
   }

   QMenu menu(dataWidget);

   QMenu * catMenu = menu.addMenu(QIcon(":/icons/category-16"), "Category...");
   if (newCatAct->isEnabled()) {
      catMenu->addAction(newCatAct);
   }
   if (deleteCatAct->isEnabled()) {
      catMenu->addAction(deleteCatAct);
   }
   if (editCatAct->isEnabled()) {
      catMenu->addAction(editCatAct);
   }

   QMenu * objMenu = menu.addMenu(QIcon(":/icons/object-16"), "Object...");
   if (newObjAct->isEnabled()) {
      objMenu->addAction(newObjAct);
   }
   if (deleteObjAct->isEnabled()) {
      objMenu->addAction(deleteObjAct);
   }
   if (editObjAct->isEnabled()) {
      objMenu->addAction(editObjAct);
   }
   objMenu->addSeparator();
   objMenu->addAction(sortByIDAct);
   objMenu->addAction(sortByFNAct);

   if (newBoxAct->isEnabled()) {
      menu.addAction(newBoxAct);
   }
   if (newKeyboxAct->isEnabled()) {
      menu.addAction(newKeyboxAct);
   }
   if (deleteBoxAct->isEnabled()) {
      menu.addAction(deleteBoxAct);
   }

   menu.exec(QCursor::pos());
}

/** This involves creating the data and video widgets and all actions, menus,
  * toolbars etc...
  */
void MainWindow::initGUI() {
   dataWidget = new DataWidget();
   videoWidget = new VideoWidget(dataWidget);

   createIcons();
   createAboutDialog();
   createActions();
   createMenus();
   createToolbars();
   createDockWidgets();
   createCentralWidget();
   createStatusBar();

   // current frame
   connect(slider, SIGNAL(valueChanged(int)), videoWidget, SLOT(seek(int))/*, Qt::QueuedConnection*/);
   connect(slider, SIGNAL(valueChanged(int)), dataWidget, SLOT(setCurrentFrame(int)));
   connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateSeekLabel(int)));
   connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(updateSeekLabel(int)));
   connect(videoWidget, SIGNAL(currentFrameChanged(int)), slider, SLOT(setValue(int)));
   connect(dataWidget, SIGNAL(currentFrameChanged(int)), slider, SLOT(setValue(int)));

   // selected object
   connect(videoWidget, SIGNAL(selectionChanged(int)), dataWidget, SLOT(setSelectedObject(int)));
   connect(dataWidget, SIGNAL(selectedObjectChanged(int)), videoWidget, SLOT(changeSelection(int)));

   connect(videoWidget, SIGNAL(updateActions()), this, SLOT(updateActions()));
   connect(dataWidget, SIGNAL(updateActions()), this, SLOT(updateActions()));

   connect(dataWidget, SIGNAL(categoryCountChanged(int)), this, SLOT(categoryCountChanged(int)));

   connect(dataWidget, SIGNAL(requestVideo(QString)), videoWidget, SLOT(openRequest(QString)));

   connect(dataWidget, SIGNAL(dataDecreased()), videoWidget, SLOT(updateData()));

   connect(videoWidget, SIGNAL(zoomChanged(float)), this, SLOT(zoomChanged(float)));

   connect(videoWidget, SIGNAL(maxFramesChanged(int)), this, SLOT(changeMaxFrames(int)));

   connect(dataWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(dataContextMenu(QPoint)));
}

/** The type of the currnetly selected bounding box and whether or not a object
  * is selected get asked from the user and the actions get updated accordingly.
  */
void MainWindow::updateActions() {
   const bool objectSelected = dataWidget->isObjectSelected();
   deleteObjAct->setEnabled(objectSelected);
   editObjAct->setEnabled(objectSelected);

   if (objectSelected) {
      const BBox::Type type = dataWidget->getCurrentBBoxType();
      newBoxAct->setEnabled(type!=BBox::SINGLE);
      newKeyboxAct->setEnabled(type!=BBox::KEYBOX);
      //deleteBoxAct->setEnabled(type==BBox::SINGLE || type==BBox::KEYBOX);
      if (type) {
         newBoxAct->setIcon(convertSingleBoxIcon);
         newBoxAct->setText(tr("Convert to b. box"));

         newKeyboxAct->setIcon(convertKeyBoxIcon);
         newKeyboxAct->setText(tr("Convert to b. box (key)"));
      }
      else {
         newBoxAct->setIcon(newSingleBoxIcon);
         newBoxAct->setText(tr("New bounding box"));

         newKeyboxAct->setIcon(newKeyBoxIcon);
         newKeyboxAct->setText(tr("New bounding box (key)"));
      }
   }
   else {
      newBoxAct->setEnabled(false);
      newBoxAct->setIcon(newSingleBoxIcon);
      newBoxAct->setText(tr("New bounding box"));
      newKeyboxAct->setEnabled(false);
      newKeyboxAct->setIcon(newKeyBoxIcon);
      newKeyboxAct->setText(tr("New bounding box (key)"));
      //deleteBoxAct->setEnabled(false);
   }
}

/** The \ref timeLabel gets updated, too.
  */
void MainWindow::updateSeekLabel(int n) {
   seekLabel->setText(QString(" Frame %1/%2").arg(n).arg(slider->maximum()));
   if (videoWidget->getFramerate() != 0.0) {
      QTime time = QTime(0, 0, 0, 0).addMSecs(n*1000/videoWidget->getFramerate());
      timeLabel->setText(" " + time.toString());
   }
   else {
      timeLabel->setText(QString());
   }
}

void MainWindow::zoomChanged(float zoom) {
   zoomLabel->setText(QString("Zoom: %1\% ").arg(qRound(zoom*100)));
}
