#ifndef TYPES_H
#define TYPES_H

#include <QtCore/QString>
#include <QtCore/QRect>
#include <QtCore/QSize>
#include <QtCore/QList>

class Object;
class QDomElement;
/// Represents a bounding box with its properties like type, framenumber and geometry.
/** This struct is the core element of the data structure to save the tracking
  * data, where multiple BBoxes form one Object. It contains the bounding box'
  * geometry in #rect, the number of the frame it belongs to in #framenumber,
  * it's type in #type and for convenience the id of its parent object in
  * #objectID.
  */
struct BBox {
   /// Type of a bounding box
   /** The type specifies whether the box really exists, got interpolated or is
     * null.
     */
   enum Type {
      NULLTYPE=0, ///< Invalid bounding box (box isn't existing).
      SINGLE  =1, ///< Normal bounding box only present for one frame.
      KEYBOX  =2, ///< Like a keyframe; Not existing bounding boxes before this box will be interpolated.
      VIRTUAL =3  ///< Interpolated bounding box.
   };
   Type type;       ///< Type of the bounding box
   int framenumber; ///< Number of the frame the bounding box appears
   QRect rect;      ///< Geometry of the bounding box
   int objectID;    ///< ID of the object the bounding box belongs to

   /// Constructs a NULL bounding box.
   BBox();
   /// Constructs a valid bounding box.
   BBox(int framenumber, QRect const & rect, int objectID, Type type=SINGLE);
   /// Compares two BBoxes using only the framenumbers.
   bool operator==(BBox const & bbox) const;
};

/// Retruns a box interpolated between two specified boxes using the framenumbers.
/** @relates BBox */
BBox interpolate(int framenumber, BBox const & bboxA, BBox const & bboxB);

/// Returns a list of interpolated BBoxes between specified framenumbers..
/** @relates BBox */
QList<BBox> interpolate(int frameStart, int frameEnd, BBox const & bboxA, BBox const & bboxB);

/// Header data of a video file bundled for interchange.
/** This struct exists to simply get the video information needed to export
  * viper files from the class holding the video data (GLWidget) to the class
  * holding the tracking data (TrackingdataWidget).
  */
struct VideofileInfo {
   QString filename; ///< Name of the file
   int framecount;   ///< Number of frames contained in the video
   QSize size;       ///< Resolution of the video

   /// Default c'tor.
   VideofileInfo();
   /// Simple constructor to initialise the struct
   VideofileInfo(QString const & name, int n, QSize size);
};

#endif // TYPES_H
