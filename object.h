#ifndef OBJECT_H
#define OBJECT_H

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QRect>
#include <QtXml/QDomElement>
#include "types.h"

/// Represents a object in the video consisting of several \ref BBox "BBox"es.
/** In detail the class only consists of a unique ID given at creation and a
  * QMap of BBox instances.
  */
class Object : public QObject {

   Q_OBJECT

public:
   /// Creates an empty object.
   Object();
   /// Creates an object from a stream
   explicit Object(QDataStream & in);
   /// Creates an object from a <a href="http://qt-project.org/doc/qt-4.8/qdomelement.html">QDomElement</a> containing an object node from a viper file.
   explicit Object(QDomElement const & objectElem);
   /// Adds the bounding box \a bbox to the internal list.
   void addBBox(BBox const & bbox);
   /// Removes the bounding box with the given \a framenumber from the internal list.
   void deleteBBoxAt(int framenumber);
   /// Getter for #id.
   int getID() const;
   /// Returns a bounding box for the specified \a framenumber.
   BBox getBBox(int framenumber) const;
   /// Returns a pointer to the bounding box for the specified \a framenumber.
   BBox * getBBoxPointer(int framenumber);
   /// Returns a pointer to the bounding box preceding the box with the given \a framenumber.
   BBox * getPrecedingBBoxPointer(int framenumber);
   /// Getter for #bboxes.
   QMap<int, BBox> const & getBBoxes() const;
   /// Returns true if the object doesn't contain any bounding boxes.
   bool isEmpty() const;
   /// Returns a <a href="http://qt-project.org/doc/qt-4.8/qdomelement.html">QDomElement</a> containing the object in the viper format.
   QDomElement toViperNode(QDomDocument & doc, QString const & catName) const;
   /// Returns a reference to the first existing bounding box
   BBox const & firstBBox() const;
   /// Returns a reference to the last existing bounding box
   BBox const & lastBBox() const;
   /// Saves the data to a stream
   void save(QDataStream & out) const;

signals:
   /// Gets emitted when a bbox gets added directly.
   /** This is used to inform the wrapping category about changes made directly
     * to this class.
     */
   void dataChanged(int objectID, int framenumber);

private:
   int id;                 ///< The unique ID of the object.
   QMap<int, BBox> bboxes; ///< The list of bounding boxes.

   /// Returns the framespan of the object as a string.
   QString getViperFramespan() const;
};

/// Compares two objects by their IDs
bool lessThanByID(Object const * const object1, Object const * const object2);
/// Compares two objects by their framenumbers
bool lessThanByFN(Object const * const object1, Object const * const object2);

#endif // OBJECT_H
