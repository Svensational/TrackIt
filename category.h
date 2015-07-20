#ifndef CATEGORY_H
#define CATEGORY_H

#include <QtCore/QAbstractTableModel>
#include "types.h"

class Object;

/// Represents a category of \ref Object "Object"s like "Person" or "Car".
/** The class as a named list of \ref Object "Object"s is derived from
  * <a href="http://qt-project.org/doc/qt-4.8/qabstractlistmodel.html">
  * QAbstractListModel</a> so it can be easily interacted with via a
  * <a href="http://qt-project.org/doc/qt-4.8/qlistview.html">QListView</a> on
  * the GUI.
  */
class Category : public QAbstractTableModel {

   Q_OBJECT

public:
   /// Constructs a category with the specified \a name
   explicit Category(QString const & name, QObject * parent = 0);
   /// Reads a category from a given stream \a in
   explicit Category(QDataStream & in, QObject * parent = 0);
   /// Deletes all objects managed by the category
   ~Category();
   /// Returns the number of objects in the list
   int rowCount(QModelIndex const & parent  = QModelIndex()) const;
   /// Returns the width of the data
   int columnCount(QModelIndex const & parent  = QModelIndex()) const;
   /// Sets the width of the data
   void setColumnCount(int n);
   /// Returns the data stored under the given \a role for the item referred to by the \a index.
   QVariant data(QModelIndex const & index, int role = Qt::DisplayRole) const;
   /// Returns the data for the given \a role and \a section in the header with the specified \a orientation.
   QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
   /// Setter for the categorie's #name
   void setName(QString const & newName);
   /// Getter for the categorie's #name
   QString const & getName() const;
   /// Returns the bounding boxes of all objects for the specified \a framenumber.
   QList<BBox> getBBoxes(int framenumber) const;
   /// Returns a pointer to the object with the specified \a ID.
   Object * getObject(int id);
   /// Getter for the stored #objects
   QList<Object *> const & getObjects() const;
   /// Returns the row of the object with the given \a id
   int findObject(int id) const;
   /// Returns whether the category contains no objects
   bool isEmpty() const;
   /// Adds the \a object to the internal list
   void addObject(Object * object);
   /// Adds the \a object to the internal list
   Category & operator<<(Object * object);
   /// Creates a new object and adds it to the internal list.
   void newObject();
   /// Deletes the object in the specified \a row.
   void deleteObjectAt(int row);
   /// Takes the object from the specified \a row.
   Object * takeObjectAt(int row);
   /// Deletes the BBoxes specified in the given \a selection
   void deleteBBoxes(QModelIndexList const & selection);
   /// Sorts the objects by their ID
   void sortByID();
   /// Sorts the objects by their framenumber
   void sortByFN();
   /// Saves the category to a given stream \a out
   void save(QDataStream & out) const;
   /// Loads the category from a given stream \a in
   void load(QDataStream & in);
   /// Returns the overall framecount of the category
   int getFramecount() const;

private:
   int columncount;         ///< The column count read from a video file
   QString name;            ///< The name of the category.
   QList<Object *> objects; ///< The list of objects.

private slots:
   /// Internal slot for change feedback from objects
   void objectDataChanged(int objectID, int framenumber);
};

#endif // CATEGORY_H
