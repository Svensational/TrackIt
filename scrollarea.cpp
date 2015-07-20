#include "scrollarea.h"
#include <QtGui/QResizeEvent>
#include <QtGui/QScrollBar>

ScrollArea::ScrollArea(QWidget * parent) :
   QScrollArea(parent)
{
   setMouseTracking(true);
}

/** If the alt key and left mouse button is pressed the scrollarea's scrollbars
  * get moved with the mouse.
  */
void ScrollArea::mouseMoveEvent(QMouseEvent * event) {
   QPoint delta = event->pos() - lastPos;

   if (event->modifiers()&Qt::ControlModifier && event->buttons()&Qt::LeftButton) {
      horizontalScrollBar()->setValue(horizontalScrollBar()->value()-delta.x());
      verticalScrollBar()->setValue(verticalScrollBar()->value()-delta.y());
   }

   lastPos = event->pos();
}

void ScrollArea::mousePressEvent(QMouseEvent * event) {
   lastPos = event->pos();
}

void ScrollArea::resizeEvent(QResizeEvent * event) {
   emit sizeChanged(event->size());
   QScrollArea::resizeEvent(event);
}
