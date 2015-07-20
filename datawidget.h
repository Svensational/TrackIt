#ifndef DATAWIDGET_H
#define DATAWIDGET_H

#include <QtGui/QTabWidget>
#include <QtGui/QItemSelection>
#include "types.h"

class Category;
class Object;
class QFile;
class QAbstractButton;
class QButtonGroup;

/// Class managing the tracking data and it's representation as a QTabWidget.
/** The Tabwidget can load and save tracking data and adds a tab containing a
  * QListView for each Category in this data. Since the Category class derives
  * from QAbstractListModel the ListView is used to represent the \ref Object
  * "Object"s in the Category.
  */
class DataWidget : public QTabWidget {

   /// Structure to represent a cell in a specific tab.
   /**
    * The struct is mainly used as a return type.
    */
   struct Cell {
      /// Tab index
      int index;
      /// Table row
      int row;
      /// Table column
      int column;
      /// Default c'tor
      /**
       * Creates a invalid Cell (all attributes have the value -1)
       */
      Cell() : index(-1), row(-1), column(-1) {}
      /// c'tor that takes a index and a model index for convenience
      /**
       * \ref row and \ref column get extracted from the \a modelIndex
       */
      Cell(int index, QModelIndex const & modelIndex) : index(index), row(modelIndex.row()), column(modelIndex.column()) {}
      /// Checks if the Cell is invalid.
      /**
       * A Cell is invalid if any of it's attributes are negative.
       */
      bool isNull() {return index<0 || row<0 || column<0;}
   };

   Q_OBJECT

public:
   /// Default c'tor, initializes everything.
   explicit DataWidget(QWidget * parent = 0);
   /// Deletes all the tracking data
   ~DataWidget();
   /// Returns the bounding boxes of all objects of all categories for the specified \a framenumber.
   QList<BBox> getBBoxes(int framenumber) const;
   /// Returns a pointer to the object with the specified \a ID.
   Object * getObject(int id) const;
   /// Setter for the \ref videofileInfo
   void setVFInfo(VideofileInfo const & newVideofileInfo);
   /// Returns the recommended minimum size for the widget.
   virtual QSize minimumSizeHint() const;
   /// Returns if a object is selected
   bool isObjectSelected() const;
   /// Returns the type of the current BBox
   BBox::Type getCurrentBBoxType() const;
   /// Sets the selection to the specified \a row
   void setSelectedObjectByRow(int row);

signals:
   /// Gets emitted when tracking data gets deleted.
   /** Other classes holding pointers to any kind of tracking data should
     * re-request them to prevent dead pointers.
     */
   void dataDecreased();
   /// Gets emitted when another object gets selected and submits its \a ID.
   /** This is part of the mechanism to keep the selection in different views
     * consistent.
     * @sa void setSelectedObject(int id)
     * @sa void VideoWidget::selectionChanged(int id)
     * @sa void VideoWidget::changeSelection(int id)
     */
   void selectedObjectChanged(int id);
   /// Gets emitted when the current \a framenumber changes.
   /** This is part of the mechanism to keep the selection in different views
     * consistent.
     * @sa void setCurrentFrame(int framenumber)
     * @sa void VideoWidget::currentFrameChanged(int n);
     * @sa void VideoWidget::seek(int frame)
     */
   void currentFrameChanged(int framenumber);
   /// Gets emitted when a data file containing information about a associated video is opened.
   /** The \a filename of the video gets submitted so the VideoWidget can try
     * to open it.
     * @sa void VideoWidget::openRequest(QString openFilename)
     */
   void requestVideo(QString filename);
   /// Gets emitted when the number of categories changes in any way.
   /** The \a count can be used to adapt the availability of actions that require
     * at least one category.
     * @sa void MainWindow::categoryCountChanged(int count)
     */
   void categoryCountChanged(int count);
   /// Gets emitted whenever the selection changes.
   /** This is used so the MainWindow can adapt the availability of selection
     * dependent actions.
     * @sa void VideoWidget::updateActions()
     * @sa void MainWindow::updateActions()
     */
   void updateActions();

public slots:
   /// Opens a data file
   void openFile();
   /// Saves the current data to a file
   void saveFile();
   /// Saves the current data to a file
   void saveFileAs();
   /// Imports data from a file in a foreign format
   void importFile();
   /// Exports data to a file in a foreign format
   void exportFile();
   /// Creates a new category and adds it to the internal list.
   void newCategory();
   /// Deletes the currently shown category
   void deleteCategory();
   /// Enables the user to edit the current category
   void editCategory();
   /// Sorts the tracking data by ID
   void sortByID();
   /// Sorts the tracking data by framenumber
   void sortByFN();
   /// Creates a new object and adds it to the currently shown category.
   void newObject();
   /// Deletes the currently selected object.
   void deleteObject();
   /// Enables the user to move objects to another category.
   void editObject();
   /// Deletes the selected bounding boxes.
   void deleteBBox();
   /// Deletes all tracking data
   void clearData();
   /// Changes the selection to the object with the specified \a ID.
   void setSelectedObject(int id);
   /// Sets the selection to the cell with the specified \a framenumber.
   void setCurrentFrame(int framenumber);
   /// Selects the next keyframe of the current object.
   void selectNextKeyframe();
   /// Selects the previous keyframe of the current object.
   void selectPreviousKeyframe();
   /// Selects the next object.
   void selectNextObject();
   /// Selects the previous object.
   void selectPreviousObject();
   /// Selects the next category.
   void selectNextCategory();
   /// Selects the previous category.
   void selectPreviousCategory();

private:
   int zoom;                     ///< The width of one box in the data view.
   int currentFrameNr;           ///< Number of the currently selected frame.
   int selectedObjectID;         ///< ID of the currently selected object.
   QString filename;             ///< Filename of the current data file.
   VideofileInfo videofileInfo;  ///< VideoFileInfo struct from the current video file
   QList<Category *> categories; ///< The list of categories
   QButtonGroup * closeBtnGroup; ///< Group to organize the close category buttons
   QButtonGroup * editBtnGroup;  ///< Group to organize the edit category buttons

   /// Deletes all tracking data without further warning
   void clearDataImmediate();
   /// Adds the given category \a cat to the internal list
   void addCategory(Category * cat);
   /// Creates and adds a category with the specified \a name and returns a pointer to it.
   Category * addCategory(QString const & name);
   /// Deletes the category with the given \a index
   void deleteCategory(int index);
   /// Enables the user to edit the category with the given \a index
   void editCategory(int index);
   /// Adds the \a object to the category specified by \a catName.
   void addObject(Object * object, QString const & catName);
   /// Imports tracking data from a ViPER file
   void importViperFile(QFile & file);
   /// Imports tracking data from a BB file
   void importBBFile(QFile & file);
   /// Exports tracking data as a ViPER file
   void exportViperFile(QFile & file);
   /// Exports tracking data as a BB file
   void exportBBFile(QFile & file);
   /// Sets the selection
   void setSelection(int tab, int row, int column);
   /// Determines the current selection (single cell) and returns it
   inline Cell getSelection() const;
   /// Creates the special last tab that creates a new category when it gets focus.
   void createNewCategoryTab();
   /// Creates the TableViews corner widget.
   void createCornerWidget();
   /// Returns the rows contained in the current selection
   QList<int> getSelectedRows() const;
   /// Determines and sets the maximum framecount of all objects.
   void updateFramecount();

private slots:
   /// Adapter from the selectionChanged Signal from the ListView to the one from this class.
   void selectionChanged(QModelIndex const & current, QModelIndex const & previous);
   /// Slot to delete the category associated with the given \a button
   void deleteCategory(QAbstractButton * button);
   /// Slot to edit the category associated with the given \a button
   void editCategory(QAbstractButton * button);
   /// Gets called when the current tab changes to \a index
   void onCurrentTabChanged(int index);
   /// Called to change the zoomlevel of the TableView to \a newZoom
   void changeZoom(int newZoom);
};

/// Creates a inactive pixmap of the given ressource \a name
QPixmap inactivePixmap(QString const & name);

#endif // DATAWIDGET_H
