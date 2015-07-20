#include "types.h"
#include <QtXml/QDomElement>

/** The type is initialised to #NULLTYPE, the other members are set to alike
  * values
  */
BBox::BBox() :
   type(NULLTYPE),
   framenumber(-1),
   rect(QRect()),
   objectID(-1)
{
}

/** The members get initialised according to the same named parameters. No more,
  * no less.
  */
BBox::BBox(int framenumber, QRect const & rect, int objectID, Type type) :
   type(type),
   framenumber(framenumber),
   rect(rect),
   objectID(objectID)
{
}

/** This operator should be used to determine whether a bounding box allready
  * exists for a specific frame or not, regardless it's geometry.
  */
bool BBox::operator==(BBox const & bbox) const {
   return (framenumber == bbox.framenumber);
}



/** @relates BBox
  * The function interpolates linear, weightet with the differences from the
  * target framenumber to the bording BBoxes framenumbers.
  */
BBox interpolate(int framenumber, BBox const & bboxA, BBox const & bboxB) {
   const qreal beta = qreal(framenumber-bboxA.framenumber)/qreal(bboxB.framenumber-bboxA.framenumber);
   const qreal alpha = 1.0-beta;
   return BBox(framenumber,
               // deprecated due to rounding errors
               //QRect(alpha*bboxA.rect.topLeft()+beta*bboxB.rect.topLeft(),
               //      alpha*bboxA.rect.size()+beta*bboxB.rect.size()),
               // better round ourselves
               QRect(qRound(alpha*bboxA.rect.x() + beta*bboxB.rect.x()),
                     qRound(alpha*bboxA.rect.y() + beta*bboxB.rect.y()),
                     qRound(alpha*bboxA.rect.width() + beta*bboxB.rect.width()),
                     qRound(alpha*bboxA.rect.height() + beta*bboxB.rect.height())),
               bboxA.objectID,
               BBox::VIRTUAL);
}

/** @relates BBox
  * Convenience function returning a list of BBoxes interpolated between frameStart and frameEnd.
  * (BBoxA and BBoxB are not part of the returned list!) (fixme: function not tested yet)
  */
QList<BBox> interpolate(int frameStart, int frameEnd, BBox const & bboxA, BBox const & bboxB){
   if (frameStart+1 >= frameEnd){
      return QList<BBox>();
   }
   else{
     QList<BBox> l;
     for (int i=frameStart+1; i<frameEnd; i++){
         l.push_back(interpolate(i,bboxA, bboxB));
     }
     return l;
   }
}



/** Creates an empty VideofileInfo
  */
VideofileInfo::VideofileInfo() :
   filename(QString()), framecount(0), size(QSize())
{
}

VideofileInfo::VideofileInfo(QString const & name, int n, QSize size) :
   filename(name), framecount(n), size(size)
{
}
