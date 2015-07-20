#include "object.h"
#include "idcounter.h"
#include <QtCore/QDataStream>

/** The object gets assigned a unique ID so it can be identified.
  */
Object::Object() :
   QObject(), id(idCounter->getID())
{
}

/** The stream data is interpreted as an object in the BTD file format.
  */
Object::Object(QDataStream & in) :
   QObject()
{
   quint32 readID;
   in >> readID;
   id = readID;
   quint32 size;
   in >> size;
   BBox bbox;
   quint8 type;
   quint32 framenumber;
   for (quint32 i=0; i<size; ++i) {
      in >> type;
      bbox.type = (BBox::Type)type;
      in >> framenumber;
      bbox.framenumber = framenumber;
      in >> bbox.rect;
      bbox.objectID = id;
      addBBox(bbox);
   }
}

/** The ctor parses the viper object node for bounding boxes and adds them as
  * BBox instances to the internal list. Boxes that last for more than one frame
  * get converted to a BBox::SINGLE and a BBox::KEYBOX typed BBox marking the
  * beginning and the end of the box from the viper file.
  * @note The object gets assigned a new unique ID, whereas the ID from the
  * viper file gets omitted to keep the integrity of the internal ID generator.
  * @sa <a href="http://qt-project.org/doc/qt-4.8/qdomelement.html">QDomElement</a>
  *     toViperNode(QDomDocument & doc, QString const & catName) const
  */
Object::Object(QDomElement const & objectElem) :
   QObject(), id(idCounter->getID())
{
   QDomElement attributeElem = objectElem.firstChildElement(QString("attribute"));
   if (!attributeElem.isNull()) {
      QDomElement bboxElem = attributeElem.firstChildElement(QString("data:bbox"));
      int firstFrame, lastFrame;
      QRect rect;
      while (!bboxElem.isNull()) {
         // viper has 1-based framenumbers, default is 0-based!
         firstFrame = bboxElem.attribute("framespan").section(':', 0, 0).toInt()-1;
         lastFrame = bboxElem.attribute("framespan").section(':', 1, 1).toInt()-1;
         rect = QRect(bboxElem.attribute("x").toInt(),
                      bboxElem.attribute("y").toInt(),
                      bboxElem.attribute("width").toInt(),
                      bboxElem.attribute("height").toInt());
         addBBox(BBox(firstFrame, rect, id, BBox::SINGLE));
         if (lastFrame>firstFrame) {
            addBBox(BBox(lastFrame, rect, id, BBox::KEYBOX));
         }
         bboxElem = bboxElem.nextSiblingElement(QString("data:bbox"));
      }
   }
}

/** The box gets inserted so that the list of boxes stays sorted by framenumber.
  * If  a box with the given framenumber already exists it will be replaced by
  * the new box.\n
  */
void Object::addBBox(BBox const & bbox) {
   bboxes.insert(bbox.framenumber, bbox);
   emit dataChanged(id, bbox.framenumber);
}

/** If no such box exists nothing happens.
  */
void Object::deleteBBoxAt(int framenumber) {
   bboxes.remove(framenumber);
}

/** This is just a convenience function.
  * @sa BBox const & lastBBox() const
  */
BBox const & Object::firstBBox() const {
   return bboxes.constBegin().value();
}

/** If there is no box defined for this frame either a interpolated or a NULL
  * bounding box is constructed and returned, according to the surrounding
  * boxes.
  * @sa BBox * getBBox(int framenumber)
  */
BBox Object::getBBox(int framenumber) const {
   QMap<int, BBox>::const_iterator i = bboxes.lowerBound(framenumber);
   if (i!=bboxes.constEnd()) {
      if (i.key()==framenumber) {
         return i.value();
      }
      else if (i!=bboxes.constBegin() && i.value().type==BBox::KEYBOX) {
         return interpolate(framenumber, (i-1).value(), i.value());
      }
   }
   return BBox();
}

/** The box is a existing, modifiable one; instead of interpolated or NULL boxes
  * a NULL pointer is returned.
  * @sa BBox getBBox(int framenumber) const
  */
BBox * Object::getBBoxPointer(int framenumber) {
   if (bboxes.contains(framenumber)) {
      return &bboxes[framenumber];
   }
   else {
      return NULL;
   }
}

QMap<int, BBox> const & Object::getBBoxes() const {
   return bboxes;
}

int Object::getID() const {
   return id;
}

/** If no such box exists a NULL pointer is returned.
  */
BBox * Object::getPrecedingBBoxPointer(int framenumber) {
   QMap<int, BBox>::iterator i = bboxes.lowerBound(framenumber);
   if (i!=bboxes.begin()) {
      return &(i-1).value();
   }
   else {
      return NULL;
   }
}

/** The string is formatted according to the ViPER format and therefore also
  * contains all holes.
  * @note ViPER has 1-based framenumbers, default is 0-based!
  */
QString Object::getViperFramespan() const {
   if (isEmpty()) {
      return QString("0:0");
   }
   QString span = QString("%1:").arg(firstBBox().framenumber+1);

   QMap<int, BBox>::const_iterator i = bboxes.constBegin()+1;
   while (i != bboxes.constEnd()) {
      if (i.value().type==BBox::SINGLE && i.key()!=(i-1).key()+1) {
         span.append(QString("%1, %2:").arg((i-1).key()+1).arg(i.key()+1));
      }
      ++i;
   }
   span.append(QString("%1").arg((bboxes.constEnd()-1).key()+1));
   return span;
}

/** Internal simply the corresponding
  * <a href="http://qt-project.org/doc/qt-4.8/qmap.html#isEmpty">isEmpty</a>
  * function of the
  * <a href="http://qt-project.org/doc/qt-4.8/qmap.html">QMap</a>
  * containing the bounding boxes is forwarded.
  */
bool Object::isEmpty() const {
   return bboxes.isEmpty();
}

/** This is just a convenience function.
  * @sa BBox const & firstBBox() const
  */
BBox const & Object::lastBBox() const {
   return (bboxes.constEnd()-1).value();
}

/** The data is saved in the BTD file format.
  */
void Object::save(QDataStream & out) const {
   out << (quint32)id;
   out << (quint32)bboxes.size();
   foreach (BBox const & bbox, bboxes) {
      out << (quint8)bbox.type;
      out << (quint32)bbox.framenumber;
      out << bbox.rect;
   }
}

/** @note If there are virtual boxes they get saved as single boxes except for
  * stationary ones (where the two spanning boxes have the same geometry) which
  * get saved using ViPER's run length encoding.
  */
QDomElement Object::toViperNode(QDomDocument & doc, QString const & catName) const {
   QDomElement objectDE = doc.createElement("object");
   objectDE.setAttribute("framespan", getViperFramespan());
   objectDE.setAttribute("id", id);
   objectDE.setAttribute("name", catName);

   QDomElement attributeDE = doc.createElement("attribute");
   attributeDE.setAttribute("name", "BoundingBox");
   objectDE.appendChild(attributeDE);

   QMap<int, BBox>::const_iterator i = bboxes.constBegin();
   while (i != bboxes.constEnd()) {
      if (i!=bboxes.constBegin() && i.value().type == BBox::KEYBOX) {
         // output interpolated BBs
         for (int j=(i-1).key()+1; j<i.key(); ++j) {
            const BBox bbox = interpolate(j, (i-1).value(), i.value());
            QDomElement databboxDE = doc.createElement("data:bbox");
            // viper has 1-based framenumbers, default is 0-based!
            databboxDE.setAttribute("framespan", QString("%1:%1").arg(bbox.framenumber+1));
            databboxDE.setAttribute("x", bbox.rect.x());
            databboxDE.setAttribute("y", bbox.rect.y());
            databboxDE.setAttribute("width", bbox.rect.width());
            databboxDE.setAttribute("height", bbox.rect.height());
            attributeDE.appendChild(databboxDE);
         }
      }
      if ((i+1)!=bboxes.constEnd() && (i+1).value().type==BBox::KEYBOX && i.value().rect == (i+1).value().rect) {
         // Merge BBs to RLE bounding box
         QDomElement databboxDE = doc.createElement("data:bbox");
         // viper has 1-based framenumbers, default is 0-based!
         databboxDE.setAttribute("framespan", QString("%1:%2").arg(i.key()+1).arg((i+1).key()+1));
         databboxDE.setAttribute("x", i.value().rect.x());
         databboxDE.setAttribute("y", i.value().rect.y());
         databboxDE.setAttribute("width", i.value().rect.width());
         databboxDE.setAttribute("height", i.value().rect.height());
         attributeDE.appendChild(databboxDE);
         ++i;
      }
      else {
         QDomElement databboxDE = doc.createElement("data:bbox");
         // viper has 1-based framenumbers, default is 0-based!
         databboxDE.setAttribute("framespan", QString("%1:%1").arg(i.key()+1));
         databboxDE.setAttribute("x", i.value().rect.x());
         databboxDE.setAttribute("y", i.value().rect.y());
         databboxDE.setAttribute("width", i.value().rect.width());
         databboxDE.setAttribute("height", i.value().rect.height());
         attributeDE.appendChild(databboxDE);
      }
      ++i;
   }

   return objectDE;
}

/** A object counts as less than another if its ID is less than the others
  * @relates Object
  */
bool lessThanByID(Object const * const object1, Object const * const object2) {
   return object1->getID() < object2->getID();
}

/** A object counts as less than another if the framenumber of its first
  * bounding box is less than the one from the other object. If they are the same
  * the framenumbers of the last bounding box get compared.
  * @relates Object
  */
bool lessThanByFN(Object const * const object1, Object const * const object2) {
   const int framenumber1 = object1->firstBBox().framenumber;
   const int framenumber2 = object2->firstBBox().framenumber;
   if (framenumber1 == framenumber2) {
      return object1->lastBBox().framenumber < object2->lastBBox().framenumber;
   }
   else {
      return framenumber1 < framenumber2;
   }
}
