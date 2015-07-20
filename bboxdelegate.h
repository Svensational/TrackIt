#ifndef BBOXDELEGATE_H
#define BBOXDELEGATE_H

#include <QtGui/QStyledItemDelegate>
#include "types.h"

/// A delegate that renders integers as pixmaps
/** The integers get interpreted as BBox::Type and a corresponding
 *  representation gets rendered in the view. The color mapping is hard coded in
 *  \ref color and the pixmaps get cached on construction.
 */
class BBoxDelegate : public QStyledItemDelegate {

public:
   /// Default constructor
   explicit BBoxDelegate(QObject * parent = 0);
   /// Paints the data at the given \a index
   void paint(QPainter * painter,
              QStyleOptionViewItem const & option,
              QModelIndex const & index) const;
   /// Virtual size hint method
   QSize sizeHint(QStyleOptionViewItem const & option,
                  QModelIndex const & index) const;

private:
   /// Internal cache of prerendered pixmaps
   QList<QPixmap> pixmaps;

   /// Initializes the pixmap cache
   void initPixmaps();
   /// Returns the hard coded colors
   QColor color(BBox::Type type, bool light=false) const;
   /// Returns the appropriate pixmap from the cache
   QPixmap const & pixmap(int width, BBox::Type type, bool current) const;
   /// Creates a pixmap
   QPixmap createPixmap(int width, BBox::Type type, bool current) const;
};

#endif // BBOXDELEGATE_H
