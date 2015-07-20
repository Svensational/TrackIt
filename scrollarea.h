#ifndef SCROLLAREA_H
#define SCROLLAREA_H

#include <QtGui/QScrollArea>

/// An extended QScrollArea
/** This scrollarea is used to make the VideoWidget scrollable
  */
class ScrollArea : public QScrollArea {

   Q_OBJECT

public:
   /// Default c'tor.
   explicit ScrollArea(QWidget * parent = 0);

signals:
   /// Gets emitted when the size of the scrollare changes
   /** This is used to adjust the zoom of the contained VideoWidget on autozoom
     */
   void sizeChanged(QSize size);

protected:
   /// Reimplemented mouse move event
   virtual void mouseMoveEvent(QMouseEvent * event);
   /// Updates #lastPos with the current mouse position
   virtual void mousePressEvent(QMouseEvent * event);
   /// Emits #sizeChanged
   virtual void resizeEvent(QResizeEvent * event);

private:
   QPoint lastPos;   ///< Last position of the cursor
};

#endif // SCROLLAREA_H
