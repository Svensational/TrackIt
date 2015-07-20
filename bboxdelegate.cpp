#include "bboxdelegate.h"
#include <QtGui/QApplication>
#include <QtGui/QPainter>

/**
 * Initializes the internal pixmap cache \ref pixmaps.
 * @sa void initPixmaps()
 */
BBoxDelegate::BBoxDelegate(QObject * parent) :
   QStyledItemDelegate(parent)
{
   initPixmaps();
}

/**
 * The color depends on the given \a type and if it is \a light or not.
 * @sa <a href="http://colorbrewer2.org/index.php?type=qualitative&scheme=Set2&n=3">
 * colorbrewer2.org</a>
 */
QColor BBoxDelegate::color(BBox::Type type, bool light) const {
   if (light) {
      switch (type) {
      case BBox::SINGLE:
         return QColor(102, 194, 165);
      case BBox::KEYBOX:
         return QColor(252, 141, 98);
      case BBox::VIRTUAL:
         return QColor(141, 160, 203);
      default:
         return QColor();
      }
   }
   else {
      switch (type) {
      case BBox::SINGLE:
         return QColor(27, 158, 119);
      case BBox::KEYBOX:
         return QColor(217, 95, 2);
      case BBox::VIRTUAL:
         return QColor(117, 112, 179);
      default:
         return QColor();
      }
   }
}

/**
 * The pixmap is colored according to the BBs \a type and whether it's
 * \a current or not.
 */
QPixmap BBoxDelegate::createPixmap(int width, BBox::Type type, bool current) const {
   int height;
   if (type == BBox::VIRTUAL) {
      height = qMax(8, width-4);
   }
   else {
      height = qMax(12, width);
   }
   QImage image(width, height, QImage::Format_ARGB32);
   const QRgb backColor = QColor(0, 0, 0, 127).rgba();
   const QRgb frontColor = color(type, current).rgba();

   // init a transparent image
   QRgb * pixel = (QRgb *)image.bits();
   for (int i=0; i<width*height; ++i) {
      pixel[i] = QColor(0, 0, 0, 0).rgba();
   }

   for (int x=0; x<width; ++x) {
      ((QRgb *)image.scanLine(0))[x] = backColor;
      ((QRgb *)image.scanLine(height-1))[x] = backColor;
   }

   if (width <= 3) {
      for (int y=1; y<height-1; ++y) {
         ((QRgb *)image.scanLine(y))[0] = frontColor;
      }
   }
   if (width > 1) {
      for (int y=1; y<height-1; ++y) {
         ((QRgb *)image.scanLine(y))[1] = frontColor;
      }
   }
   if (width > 2) {
      for (int y=1; y<height-1; ++y) {
         ((QRgb *)image.scanLine(y))[width-1] = backColor;
      }
   }
   if (width > 3) {
      for (int y=1; y<height-1; ++y) {
         ((QRgb *)image.scanLine(y))[0] = backColor;
         ((QRgb *)image.scanLine(y))[width-2] = frontColor;
      }
   }
   if (width > 4) {
      for (int y=2; y<height-2; ++y) {
         ((QRgb *)image.scanLine(y))[width-3] = backColor;
      }
      for (int x=2; x<width-2; ++x) {
         ((QRgb *)image.scanLine(1))[x] = frontColor;
         ((QRgb *)image.scanLine(height-2))[x] = frontColor;
      }
   }
   if (width > 5) {
      for (int y=2; y<height-2; ++y) {
         ((QRgb *)image.scanLine(y))[2] = backColor;
      }
   }
   if (width > 6) {
      for (int x=3; x<width-3; ++x) {
         ((QRgb *)image.scanLine(2))[x] = backColor;
         ((QRgb *)image.scanLine(height-3))[x] = backColor;
      }
   }
   return QPixmap::fromImage(image);
}

/**
 * A Pixmap for any type, selection type and size is created.
 * @sa QPixmap createPixmap(int width, BBox::Type type, bool current) const
 */
void BBoxDelegate::initPixmaps() {
   for (int i=1; i<=16; ++i) {
      pixmaps << createPixmap(i, BBox::SINGLE, false);
      pixmaps << createPixmap(i, BBox::SINGLE, true);
      pixmaps << createPixmap(i, BBox::KEYBOX, false);
      pixmaps << createPixmap(i, BBox::KEYBOX, true);
      pixmaps << createPixmap(i, BBox::VIRTUAL, false);
      pixmaps << createPixmap(i, BBox::VIRTUAL, true);
   }
}

/**
 * If the data at the given \a index can be converted to integer it gets
 * interpreted as BBox::Type and according to this type and the given \a option
 * a representing pixmap gets painted with the \a painter. \n
 * Otherwise the default paint method from
 * <a href="http://qt-project.org/doc/qt-4.8/qstyleditemdelegate.html">
 * QStyledItemDelegate</a> is called.
 */
void BBoxDelegate::paint(QPainter * painter,
                         QStyleOptionViewItem const & option,
                         QModelIndex const & index) const {
   if (index.data().canConvert(QVariant::Int)) {
      const BBox::Type type = (BBox::Type)index.data().toInt();

      if (option.state.testFlag(QStyle::State_Selected)) {
         /*QRect rect = option.rect;
         if (rect.width() < 2) {
            rect.setWidth(rect.width()+1);
            rect.setLeft(rect.left()-1);
         }*/
         if (type != BBox::NULLTYPE) {
            painter->fillRect(option.rect,
                              color(type,
                                    option.state.testFlag(QStyle::State_HasFocus)));
         }
         else {
            painter->fillRect(option.rect,
                              option.palette.highlight());
         }
      }

      if (type != BBox::NULLTYPE) {
         QApplication::style()->drawItemPixmap(painter,
                                               option.rect,
                                               Qt::AlignCenter,
                                               pixmap(option.rect.width(),
                                                      type,
                                                      option.state.testFlag(QStyle::State_Selected)));
      }
   }
   else {
      QStyledItemDelegate::paint(painter, option, index);
   }
}

/**
 * The pixmap is chosen according to the given \a width, \a type and if it is
 * \a current (selected) or not.
 */
QPixmap const & BBoxDelegate::pixmap(int width, BBox::Type type, bool current) const {
   int i = (qBound(1, width, 16)-1)*6;
   i += (type-1)*2;
   if (current) i+=1;
   return pixmaps.at(i);
}

/**
 * Returns the size needed by the delegate to display the item specified by
 * \a index, taking into account the style information provided by \a option.
 */
QSize BBoxDelegate::sizeHint(QStyleOptionViewItem const & option,
                             QModelIndex const & index) const {
   if (index.data().canConvert(QVariant::Int)) {
      return QSize(qBound(1, option.rect.width(), 16), qBound(8, option.rect.width(), 16));
   }
   else {
      return QStyledItemDelegate::sizeHint(option, index);
   }
}
