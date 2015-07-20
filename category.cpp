#include "category.h"
#include <QtGui/QIcon>
#include "object.h"

/** Since the class derives from QAbstractListModel this ctor is called as well.
  * @sa <a href="http://qt-project.org/doc/qt-4.8/qabstractlistmodel.html#QAbstractListModel">
  *     QAbstractItemModel::QAbstractListModel(QObject * parent = 0)</a>
  */
Category::Category(QString const & name, QObject * parent) :
   QAbstractTableModel(parent), columncount(0), name(name)
{
}

/** Since the class derives from QAbstractListModel this ctor is called as well.
  * @sa <a href="http://qt-project.org/doc/qt-4.8/qabstractlistmodel.html#QAbstractListModel">
  *     QAbstractItemModel::QAbstractListModel(QObject * parent = 0)</a>
  */
Category::Category(QDataStream & in, QObject * parent) :
   QAbstractTableModel(parent), columncount(0)
{
   in >> name;
   quint32 size;
   in >> size;
   for (quint32 i=0; i<size; ++i) {
      addObject(new Object(in));
   }
}

Category::~Category() {
   qDeleteAll(objects);
   objects.clear();
}

/**
 * In fact the pointer to the \a object gets added, so the submitted pointer
 * stays valid. The category takes ownership of the \a object and deletes it if
 * necessary.
 */
void Category::addObject(Object * object) {
   if (object) {
      beginInsertRows(QModelIndex(), objects.size(), objects.size());
      objects << object;
      endInsertRows();
      connect(object, SIGNAL(dataChanged(int,int)), this, SLOT(objectDataChanged(int,int)));
   }
}

/**
 * @sa <a href="http://qt-project.org/doc/qt-4.8/qabstractitemmodel.html#columnCount">
 *     int QAbstractItemModel::columnCount(const QModelIndex & parent) const</a>
 */
int Category::columnCount(QModelIndex const & parent) const {
   if (parent.isValid()) {
      return 0;
   }
   else {
      return columncount;
   }
}

/**
 * @sa <a href="http://qt-project.org/doc/qt-4.8/qabstractitemmodel.html#data">
 *     QVariant QAbstractItemModel::data(const QModelIndex & index, int role) const</a>
 */
QVariant Category::data(QModelIndex const & index, int role) const {
   switch (role) {
   case Qt::DisplayRole:
      return objects.at(index.row())->getBBox(index.column()).type;
   case Qt::ToolTipRole:
      return QString("Frame %1").arg(index.column());
   case Qt::UserRole:
      return objects.at(index.row())->getID();
   default:
      return QVariant();
   }
}

/**
 * The deletion order gets forwarded to the corresponding objects and al connected
 * views get order to update.
 * @sa void Object::deleteBBoxAt(int framenumber)
 */
void Category::deleteBBoxes(QModelIndexList const & selection) {
   foreach (QModelIndex const & i, selection) {
      objects.at(i.row())->deleteBBoxAt(i.column());
      // virtual/null boxes before the deleted box could have changed, too.
      emit dataChanged(index(i.row(), 0), i);
   }
}

/**
 * It gets the object via takeObjectAt which disconnects all signals and deletes
 * it then.
 * @note with the object all the associated bounding boxes get lost as well.
 * @sa Object * takeObjectAt(int row)
 */
void Category::deleteObjectAt(int row) {
   delete takeObjectAt(row);
}

/**
 * If the object can't be found -1 is returned.
 */
int Category::findObject(int id) const {
   int row = -1;
   int i=0;
   while (row<0 && i<objects.size()) {
      if (objects.at(i)->getID() == id) {
         row = i;
      }
      ++i;
   }
   return row;
}

/**
 * The objects simply get looped through collecting all the currently visible
 * bounding boxes which get grouped in a QList.
 * @note Since the returned list contains copies of the boxes altering them
 * will not affect the original data.
 * @sa BBox Object::getBBox(int framenumber) const
 */
QList<BBox> Category::getBBoxes(int framenumber) const {
   QList<BBox> bboxes;
   BBox bbox;
   foreach (Object const * const object, objects) {
      bbox = object->getBBox(framenumber);
      if (bbox.type/* != BBox::NULLTYPE*/) {
         bboxes << bbox;
      }
   }
   return bboxes;
}

/**
 * This framecount is the actual maximum of all the objects last framenumbers and
 * therefor can be seen as a minimum width for views.
 */
int Category::getFramecount() const {
   int framecount = 0;
   foreach (Object const * const object, objects) {
      framecount = qMax(framecount, object->lastBBox().framenumber+1);
   }
   return framecount;
}

QString const & Category::getName() const {
   return name;
}

/**
 * If no such object exists a NULL pointer is returned instead.
 */
Object * Category::getObject(int id) {
   Object * object = NULL;
   int i=0;
   while (!object && i<objects.size()) {
      if (objects.at(i)->getID() == id) {
         object = objects.at(i);
      }
      ++i;
   }
   return object;
}

QList<Object *> const & Category::getObjects() const {
   return objects;
}

/**
 * @sa <a href="http://qt-project.org/doc/qt-4.8/qabstractitemmodel.html#headerData">
 *     QVariant QAbstractItemModel::headerData(int section, Qt::Orientation
 *     orientation, int role) const</a>
 */
QVariant Category::headerData(int section, Qt::Orientation orientation, int role) const {
   switch (role) {
   case Qt::DisplayRole:
      if (orientation == Qt::Vertical) {
         return QString("%1 %2").arg(name).arg(objects.at(section)->getID());
      }
      else {
         return section;
      }
   case Qt::DecorationRole:
      if (orientation == Qt::Vertical) {
         return QIcon(":/icons/object-12");
      }
      else {
         return QVariant();
      }
   case Qt::ToolTipRole:
      if (orientation == Qt::Horizontal) {
         return QString("Frame %1").arg(section);
      }
      else {
         return QVariant();
      }
   default:
      return QVariant();
   }
}

bool Category::isEmpty() const {
   return objects.isEmpty();
}

/**
 * The object is empty (doesn't contain any bounding boxes) and gets itself
 * a unique ID.
 */
void Category::newObject() {
   addObject(new Object());
}

/**
 * It simply is a adapter to the dataChanged signal which it emits.
 */
void Category::objectDataChanged(int objectID, int framenumber) {
   const int row = findObject(objectID);
   if (row >= 0 && framenumber >= 0) {
      emit dataChanged(index(row, 0), index(row, framenumber));
   }
}

/**
 * In fact the pointer to the \a object gets added, so the submitted pointer
 * stays valid. The category takes ownership of the \a object and deletes it if
 * necessary.
 * @sa void addObject(Object * object)
 */
Category & Category::operator <<(Object * object) {
   addObject(object);
   return (*this);
}

/**
 * @sa <a href="http://qt-project.org/doc/qt-4.8/qabstractitemmodel.html#rowCount">
 *     int QAbstractItemModel::rowCount(const QModelIndex & parent =
 *     QModelIndex()) const</a>
 */
int Category::rowCount(QModelIndex const &) const {
   return objects.size();
}

/**
 * Saves it's name and all objects.
 * @sa void Object::save(QDataStream & out) const
 */
void Category::save(QDataStream & out) const {
   out << name;
   out << (quint32)objects.size();
   foreach (Object const * const object, objects) {
      object->save(out);
   }
}

/**
 * Used to widen the attached views to match a video so even frames without boxes
 * can be selected.
 */
void Category::setColumnCount(int n) {
   emit layoutAboutToBeChanged();
   columncount = n;
   emit layoutChanged();
}

void Category::setName(QString const & newName) {
   name = newName;
}

/**
 * The sort is stable and uses the corresponding sort function from class Object
 * @sa bool lessThanByFN(Object const * const object1, Object const * const object2)
 */
void Category::sortByFN() {
   beginResetModel();
   //emit layoutAboutToBeChanged();
   qStableSort(objects.begin(), objects.end(), lessThanByFN);
   endResetModel();
   //emit layoutChanged();
}

/**
 * The sort is stable and uses the corresponding sort function from class Object
 * @sa bool lessThanByID(Object const * const object1, Object const * const object2)
 */
void Category::sortByID() {
   beginResetModel();
   //emit layoutAboutToBeChanged();
   qSort(objects.begin(), objects.end(), lessThanByID);
   endResetModel();
   //emit layoutChanged();
}

/**
 * The connected signals get disconnected and the category ends it's ownership
 */
Object * Category::takeObjectAt(int row) {
   beginRemoveRows(QModelIndex(), row, row);
   Object * object = objects.takeAt(row);
   endRemoveRows();
   disconnect(object, SIGNAL(dataChanged(int,int)), this, SLOT(objectDataChanged(int,int)));
   return object;
}
