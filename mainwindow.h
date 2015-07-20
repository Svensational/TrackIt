#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

class VideoWidget;
class DataWidget;
class QToolButton;
class QSlider;
class QLabel;
class QShortcut;
class Julia;

/// Class managing the GUI and connecting the other classes.
/** The MainWindow holds all the QActions used for user interaction and
  * instances and connects the TrackingdataWidget and GLWidget.
  */
class MainWindow : public QMainWindow {

   Q_OBJECT

public:
   /// Default ctor
   explicit MainWindow(QWidget * parent = 0);
   /// Default dtor
   ~MainWindow();

private:
   VideoWidget * videoWidget;       ///< The GLWidget instance
   DataWidget * dataWidget;         ///< The TrackingdataWidget instance
   QSlider * slider;                ///< The Slider for seeking in the video
   QAction * openVideoAct;          ///< Action to open a video file
   QAction * openDataAct;           ///< Action to open a data file
   QAction * saveDataAct;           ///< Action to save a data file
   QAction * saveDataAsAct;         ///< Action to save a data file under a specific filename
   QAction * importDataAct;         ///< Action to import a data file
   QAction * exportDataAct;         ///< Action to export a data file
   QAction * quitAct;               ///< Action to quit the application
   QAction * playPauseAct;          ///< Action to start/pause video playback
   QAction * nextFrameAct;          ///< Action to seek forward
   QAction * previousFrameAct;      ///< Action to seek backward
   QAction * nextFrameKeyAct;       ///< Action to seek forward until keybox
   QAction * previousFrameKeyAct;   ///< Action to seek backward until keybox
   QAction * nextCategoryAct;       ///< Action to select the next category
   QAction * previousCategoryAct;   ///< Action to select the previous category
   QAction * nextObjectAct;         ///< Action to select the next object
   QAction * previousObjectAct;     ///< Action to select the previous object
   QAction * clearDataAct;          ///< Action to clear all the tracking data
   QAction * newBoxAct;             ///< Action to create a new bounding box
   QAction * newKeyboxAct;          ///< Action to create a new key bounding box
   QAction * deleteBoxAct;          ///< Action to delete a bounding box
   QAction * zoomInAct;             ///< Action to zoom in
   QAction * zoomOutAct;            ///< Action to zoom out
   QAction * zoomResetAct;          ///< Action to reset zoom
   QAction * zoomFitAct;            ///< Action to activate auto zoom
   QAction * newCatAct;             ///< Action to create a new category
   QAction * deleteCatAct;          ///< Action to delete a category
   QAction * editCatAct;            ///< Action to rename a category
   QAction * sortByIDAct;           ///< Action to sort all data by object ID
   QAction * sortByFNAct;           ///< Action to sort all data by appearence
   QAction * newObjAct;             ///< Action to create a new object
   QAction * deleteObjAct;          ///< Action to delete a object
   QAction * editObjAct;            ///< Action to edit a object
   QAction * toggleCacheAct;			///< Action to switch cache on or off
   QAction * toggleCenterlineAct;	///< Action to switch centerline visibility on or off
   QAction * setCacheProperties;    ///< Action to manipulate the cache
   QAction * aboutAction;           ///< Action to show a about dialog
   QLabel * seekLabel;              ///< The label shows the current framenumber
   QLabel * zoomLabel;              ///< The label shows the current zoom factor
   QLabel * timeLabel;              ///< The label shows the elapsed time
   QIcon newSingleBoxIcon;          ///< Icon for a the new single box action
   QIcon newKeyBoxIcon;             ///< Icon for a the new key box action
   QIcon convertSingleBoxIcon;      ///< Icon for a the convert to single box action
   QIcon convertKeyBoxIcon;         ///< Icon for a the convert to key box action
   QDialog * aboutDialog;           ///< The about dialog
   /// @cond
   Julia * julia;
   QShortcut * juliaShortcut;
   /// @endcond

   /// Inits the GUI
   void initGUI();
   /// Creates the Actions
   void createActions();
   /// Creates the Menus
   void createMenus();
   /// Creates the Toolbars
   void createToolbars();
   /// Creates the DockWidgets
   void createDockWidgets();
   /// Creates the central widget (VideoWidget)
   void createCentralWidget();
   /// Creates the status bar
   void createStatusBar();
   /// Creates the about dialog
   void createAboutDialog();
   /// Creates the layout for the zoom actions
   QLayout * createZoomLayout();
   /// Creates the 'new' and 'convert' icons
   void createIcons();

private slots:
   /// Updates the \ref zoomLabel
   void zoomChanged(float zoom);
   /// Updates the \ref seekLabel
   void updateSeekLabel(int n);
   /// Updates the seek \ref slider range
   void changeMaxFrames(int n);
   /// Creates and shows the data widgets context menu
   void dataContextMenu(QPoint const & pos);
   /// Updates selection dependent actions
   void categoryCountChanged(int count);
   /// Updates selection dependent actions
   void updateActions();
};

#endif // MAINWINDOW_H
