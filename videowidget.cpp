#include "videowidget.h"
#include <iostream>
#include <QtCore/QTimer>
#include <QtGui/QFileDialog>
#include <QtGui/QFormLayout>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QWheelEvent>
#include <opencv2/core/core.hpp>
#include <GL/glext.h>
#include "datawidget.h"
#include "object.h"



inline int getNearestPOT(int n) {
   int m = 1;
   while (m<n) {
      m*=2;
   }
   return m;
}

VideoWidget::VideoWidget(DataWidget * data, QWidget * parent) :
   QGLWidget(parent),
   autoZoom(true),
   showCenterLines(true),
   createBoxFlag(false),
   createKeyboxFlag(false),
   videoSize(QSize()),
   zoom(1.0),
   texCoords(QSizeF(1.0, 1.0)),
   currentTexture(0),
   currentFrame(0),
   selectedObj(NULL),
   selectedBBox(NULL),
   hitArea(NONE),
   data(data),
   cacheEnabled(true),
   cacheSize(45)
{
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   timer = new QTimer(this);
   connect(timer, SIGNAL(timeout()), this, SLOT(showNextFrame()));
   setMouseTracking(true);
}

/** The framerate is obtained from the openCV \ref capture
  */
double VideoWidget::getFramerate() {
   return capture.get(CV_CAP_PROP_FPS);
}

/** The corresponding object is retrieved from the DataWidget. The \ref
 * selectedBBox is updated as well.
 * \sa void selectionChanged(int id)
 */
void VideoWidget::changeSelection(int id) {
   if (!selectedObj || selectedObj->getID()!=id) {
      selectedObj = data->getObject(id);
      if (selectedObj) {
         selectedBBox = selectedObj->getBBoxPointer(currentFrame);
      }
      else {
         selectedBBox = NULL;
      }
      updateGL();
   }
}

/** Internally the \ref createBoxFlag is set and the cursor is set to a
  * crosshair. If a virtual box exists it gets converted to a single box instead
  * and if a box exists for the last frame it gets copied instead, so it should
  * be easier to create consistent boxes.
  * \sa void createKeyBBox()
  */
void VideoWidget::createBBox() {
   if (!selectedObj || createBoxFlag) {
      // cancel already started creation
      createBoxFlag = false;
      unsetCursor();
      emit boxCreationStarted(false);
      return;
   }
   BBox currentBBox = selectedObj->getBBox(currentFrame);
   if (currentBBox.type) {
      // it's a existing box
      currentBBox.type = BBox::SINGLE;
      selectedObj->addBBox(currentBBox);
      selectedBBox = selectedObj->getBBoxPointer(currentFrame);
      emit boxCreationStarted(false);
      emit updateActions();
   }
   else {
      // no box present
      BBox * previousBBox = selectedObj->getBBoxPointer(currentFrame-1);
      if (previousBBox) {
         // preceding box present
         selectedObj->addBBox(BBox(currentFrame,
                                   previousBBox->rect,
                                   selectedObj->getID(),
                                   BBox::SINGLE));
         selectedBBox = selectedObj->getBBoxPointer(currentFrame);
         emit boxCreationStarted(false);
         emit updateActions();
      }
      else {
         // really nothing present
         createBoxFlag = true;
         setCursor(Qt::CrossCursor);
      }
   }
   updateGL();
}

/** Internally the \ref createKeyboxFlag is set and the cursor is set to a
  * crosshair. If a virtual box exists it gets converted to a key box instead
  * and if a box exists before it gets copied instead.
  * \sa void createBBox()
  */
void VideoWidget::createKeyBBox() {
   if (!selectedObj || createKeyboxFlag) {
      createKeyboxFlag = false;
      unsetCursor();
      emit boxCreationStarted(false);
      return;
   }
   BBox currentBBox = selectedObj->getBBox(currentFrame);
   if (currentBBox.type) {
      // it's a existing box
      currentBBox.type = BBox::KEYBOX;
      selectedObj->addBBox(currentBBox);
      selectedBBox = selectedObj->getBBoxPointer(currentFrame);
      emit boxCreationStarted(false);
      emit updateActions();
   }
   else {
      // no box present
      BBox * previousBBox = selectedObj->getPrecedingBBoxPointer(currentFrame);
      if (previousBBox) {
         // preceding box present
         selectedObj->addBBox(BBox(currentFrame,
                                   previousBBox->rect,
                                   selectedObj->getID(),
                                   BBox::KEYBOX));
         selectedBBox = selectedObj->getBBoxPointer(currentFrame);
         emit boxCreationStarted(false);
         emit updateActions();
      }
      else {
         // really nothing present
         createKeyboxFlag = true;
         setCursor(Qt::CrossCursor);
      }
   }
   updateGL();
}

/** The GL states are set and a default projection is defined.
 */
void VideoWidget::initializeGL() {
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_POINT_SMOOTH);
   glEnable(GL_LINE_SMOOTH);
   glLineStipple(1, 0xF0F0);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0.0, 1.0, 1.0, 0.0, -5.0, 5.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

/** Internally a small QRect is created, moved to every handles position and
 * checked for containment of the \a pos
 * \note The \a pos has to be in video coordinates!
 */
VideoWidget::Hitarea VideoWidget::isHit(QRect const & rect, QPoint const & pos) const {
   QRect handle(QPoint(0, 0), QSize(9, 9)/zoom);

   handle.moveCenter(rect.topLeft());
   if (handle.contains(pos)) {
      return TOPLEFT;
   }
   handle.moveCenter(rect.topRight());
   if (handle.contains(pos)) {
      return TOPRIGHT;
   }
   handle.moveCenter(rect.bottomLeft());
   if (handle.contains(pos)) {
      return BOTTOMLEFT;
   }
   handle.moveCenter(rect.bottomRight());
   if (handle.contains(pos)) {
      return BOTTOMRIGHT;
   }
   handle.moveCenter(QPoint(rect.center().x(), rect.top()));
   if (handle.contains(pos)) {
      return TOP;
   }
   handle.moveCenter(QPoint(rect.left(), rect.center().y()));
   if (handle.contains(pos)) {
      return LEFT;
   }
   handle.moveCenter(QPoint(rect.right(), rect.center().y()));
   if (handle.contains(pos)) {
      return RIGHT;
   }
   handle.moveCenter(QPoint(rect.center().x(), rect.bottom()));
   if (handle.contains(pos)) {
      return BOTTOM;
   }
   if (rect.contains(pos)) {
      return CENTER;
   }
   return NONE;
}

/** If \ref hitArea is defined the according edge/corner of the box gets moved.
  * Else the cursor gets updated via updateCursor().
  */
void VideoWidget::mouseMoveEvent(QMouseEvent * event) {
   if (selectedBBox) {
      const QPoint delta = event->pos()/zoom - hitPos;
      hitPos = event->pos()/zoom;
      switch (hitArea) {
      case TOPLEFT:
         selectedBBox->rect.setTopLeft(selectedBBox->rect.topLeft()+delta);
         break;
      case TOP:
         selectedBBox->rect.setTop(selectedBBox->rect.top()+delta.y());
         break;
      case TOPRIGHT:
         selectedBBox->rect.setTopRight(selectedBBox->rect.topRight()+delta);
         break;
      case LEFT:
         selectedBBox->rect.setLeft(selectedBBox->rect.left()+delta.x());
         break;
      case CENTER:
         selectedBBox->rect.moveCenter(selectedBBox->rect.center()+delta);
         break;
      case RIGHT:
         selectedBBox->rect.setRight(selectedBBox->rect.right()+delta.x());
         break;
      case BOTTOMLEFT:
         selectedBBox->rect.setBottomLeft(selectedBBox->rect.bottomLeft()+delta);
         break;
      case BOTTOM:
         selectedBBox->rect.setBottom(selectedBBox->rect.bottom()+delta.y());
         break;
      case BOTTOMRIGHT:
         selectedBBox->rect.setBottomRight(selectedBBox->rect.bottomRight()+delta);
         break;
      default:
         updateCursor();
         break;
      }
   }
   event->ignore();
   if (hitArea) {
      updateData();
   }
}

/** If \ref createBoxFlag or \ref createKeyboxFlag is set a new zero sized
 * bounding box gets created at the \a events position and \ref hitArea is set
 * to \ref BOTTOMRIGHT so a following move event will resize the box.\n
 * Else, if a BBox is currently selected the corresponding \ref Hitarea is
 * determined.\n
 * If a unselected BBox or nothing gets hit the \ref selectedObj and \ref
 * selectedBBox are updated.
 */
void VideoWidget::mousePressEvent(QMouseEvent * event) {
   hitArea = NONE;
   hitPos = event->pos()/zoom;

   if (createBoxFlag || createKeyboxFlag) {
      BBox bbox(currentFrame,
                QRect(hitPos, QSize(1, 1)),
                selectedObj->getID(),
                createBoxFlag?BBox::SINGLE:BBox::KEYBOX);
      selectedObj->addBBox(bbox);
      updateData();
      hitArea = BOTTOMRIGHT;
      createBoxFlag = false;
      createKeyboxFlag = false;
      emit boxCreationStarted(false);
      emit updateActions();
   }
   else {
      // check whether and where the selected BBox was hit
      if (selectedBBox) {
         hitArea = isHit(selectedBBox->rect, hitPos);
      }

      if (!hitArea) {
         bboxes = data->getBBoxes(currentFrame);
         int i = bboxes.size();
         bool hit = false;
         while (!hit && i>0) {
            hit = bboxes.at(--i).rect.contains(hitPos);
         }
         if (hit) {
            selectedObj = data->getObject(bboxes.at(i).objectID);
            selectedBBox = selectedObj->getBBoxPointer(currentFrame);
            emit selectionChanged(bboxes.at(i).objectID);
            updateCursor();
         }
         else {
            selectedObj = NULL;
            selectedBBox = NULL;
            emit selectionChanged(-1);
         }
      }
   }

   event->ignore();
   updateGL();
}

/** \ref hitArea is set to \ref NONE and created boxes get normalized
 */
void VideoWidget::mouseReleaseEvent(QMouseEvent * event) {
   hitArea = NONE;
   // correct wrong BBs
   if (selectedBBox && !selectedBBox->rect.isValid()) {
      selectedBBox->rect = selectedBBox->rect.normalized();
   }
   event->accept();
}

/** If the video doesn't exists nothing happens, else it is opened without any
  * further prompt into the openCV \ref capture and a VideofileInfo gets
  * screated and sent to the DataWidget.
  */
void VideoWidget::openRequest(QString openFilename) {
   QString filename = openFilename;
   if (!QFile::exists(filename)) {
      filename = QFileDialog::getOpenFileName(this,
                                              tr("Linked video file couldn't be found"),
                                              filename,
                                              tr("Videos (*.avi *.mpg);;All files (*.*)"));

      if (filename.isEmpty()) {
         return;
      }
   }

   capture.open(filename.toStdString());

   if (!capture.isOpened()) {
      QMessageBox::warning(this,
                           tr("Unable to open video"),
                           tr("The file\n\"")+filename+tr("\"\ncouldn't be opened as a video!"));
      return;
   }

   videoSize.setWidth(capture.get(CV_CAP_PROP_FRAME_WIDTH));
   videoSize.setHeight(capture.get(CV_CAP_PROP_FRAME_HEIGHT));
   resizeTexture();
   setZoom(1.0);
   emit maxFramesChanged(capture.get(CV_CAP_PROP_FRAME_COUNT));

   data->setVFInfo(VideofileInfo(filename.section('/', -1),
                                 capture.get(CV_CAP_PROP_FRAME_COUNT),
                                 videoSize));
   // Just to make sure everything updates
   currentFrame = -1;
   clearFrameCache();
   emit currentFrameChanged(1);
   seek(0);
}

/** The filename is asked from the user via a QFileDialog and the video is
  * opened using \ref openRequest
  */
void VideoWidget::openVideoFile() {
   QString filename = QFileDialog::getOpenFileName(this,
                                                   tr("Open video file"),
                                                   QString(),
                                                   tr("Videos (*.avi *.mpg *.mpeg *.mp4 *.m4v *.mkv *.flv *.wmv);;All files (*.*)"));

   if (filename.isEmpty()) {
      return;
   }

   QDir::setCurrent(filename.section('/', 0, -2));

   openRequest(filename);
}

/** First the current video frame gets rendered, then the currently visible
 * bounding boxes and the selected object.
 * \sa void renderCurrentFrame() const
 * \sa void renderBBox(BBox const & bbox, bool active) const
 * \sa void renderSelectedObject() const
 */
void VideoWidget::paintGL() {
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0.0, 1.0, 1.0, 0.0, -5.0, 5.0);
   glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
   renderCurrentFrame();

   glLoadIdentity();
   glOrtho(0.5f, videoSize.width()+0.5f, videoSize.height()+0.5f, 0.5f, -5.0f, 5.0f);
   foreach (BBox const & bbox, bboxes) {
      if (!selectedObj || bbox.objectID != selectedObj->getID()) {
         renderBBox(bbox);
      }
   }

   renderSelectedObject();
}

/** The \ref timer gets started with a interval according to the videos FPS and
 * \ref playToggled gets emitted with true.
 * \note the \ref timer timeout signal is connected to showNextFrame()
 */
void VideoWidget::play(bool play) {
   if (capture.isOpened()) {
      if (play) {
         timer->start(1000.0/capture.get(CV_CAP_PROP_FPS));
      }
      else {
         timer->stop();
      }
      emit playToggled(play);
   }
   else {
      emit playToggled(false);
   }
}

/** The color gets chosen according to \a active
 */
void VideoWidget::renderBBox(BBox const & bbox, bool active) const {
   // create Array holding the bbox vertices:
   int theBox[16] = { bbox.rect.left(), bbox.rect.bottom(),
                      bbox.rect.right(), bbox.rect.bottom(),
                      bbox.rect.right(), bbox.rect.top(),
                      bbox.rect.left(), bbox.rect.top(),
                      bbox.rect.left(), bbox.rect.center().y(),
                      bbox.rect.center().x(), bbox.rect.bottom(),
                      bbox.rect.right(), bbox.rect.center().y(),
                      bbox.rect.center().x(), bbox.rect.top()};

   //render outlines
   glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
   if (active) {
      glLineWidth(4.5f);
   }
   else {
      glLineWidth(3.5f);
   }
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(2, GL_INT, 0, &theBox);
   glDrawArrays(GL_LINE_LOOP, 0, 4);
   if (active && (bbox.type==BBox::SINGLE || bbox.type==BBox::KEYBOX)) {
      glPointSize(9.0f);
      glDrawArrays(GL_POINTS, 0, 8);
   }

   //render box
   switch (bbox.type) {
   case BBox::SINGLE:
      glColor4ub(102, 194, 165, 255);
      break;
   case BBox::KEYBOX:
      glColor4ub(252, 141, 98, 255);
      break;
   case BBox::VIRTUAL:
      glColor4ub(141, 160, 203, 255);
      break;
   default:
      break;
   }
   if (active) {
      glLineWidth(2.5f);
   }
   else {
      glLineWidth(1.5f);
   }
   glDrawArrays(GL_LINE_LOOP, 0, 4);
   if (active && (bbox.type==BBox::SINGLE || bbox.type==BBox::KEYBOX)) {
      glPointSize(7.0f);
      glDrawArrays(GL_POINTS, 0, 8);
   }
   glDisableClientState(GL_VERTEX_ARRAY);
}

/** The object is represented by a white line connecting the center of all of
  * its BBoxes. The selected box is also rendered as a fat box with visible
  * handles and a white point at its center to clarify its position on the white
  * line.
  * \sa void renderCenterline() const
  * \sa void renderBBox(BBox const & bbox, bool active = false) const
  */
void VideoWidget::renderSelectedObject() const {
   if (!selectedObj) {
      return;
   }
   if (showCenterLines){
      renderCenterline();
   }
   // now paint BBox
   BBox bbox = selectedObj->getBBox(currentFrame);
   if (bbox.type) {
      glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
      glPointSize(9.0f);
      glBegin(GL_POINTS);
      glVertex2f(bbox.rect.center().x(), bbox.rect.center().y());
      glEnd();

      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
      glPointSize(7.0f);
      glBegin(GL_POINTS);
      glVertex2f(bbox.rect.center().x(), bbox.rect.center().y());
      glEnd();

      renderBBox(bbox, true);
   }
}

/** The centerline connects the centers of all bounding boxes to represent an object
  */
void VideoWidget::renderCenterline() const {
   int prevFrameNumber;
   QPoint prevCenter;
   // Paint Centerlines:
   // set color for background lines
   glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
   glLineWidth(3.5f);

   for (int pass=0; pass<2; ++pass) {
      // paint solid lines
      prevFrameNumber = selectedObj->firstBBox().framenumber;
      prevCenter = selectedObj->firstBBox().rect.center();
      glBegin(GL_LINES);
      foreach (BBox const & bbox, selectedObj->getBBoxes()) {
         if (bbox.type==BBox::SINGLE && bbox.framenumber==prevFrameNumber+1) {
            glVertex2f(prevCenter.x(), prevCenter.y());
            glVertex2f(bbox.rect.center().x(), bbox.rect.center().y());
         }
         prevFrameNumber = bbox.framenumber;
         prevCenter = bbox.rect.center();
      }
      glEnd();

      // paint stippled lines
      prevFrameNumber = selectedObj->firstBBox().framenumber;
      prevCenter = selectedObj->firstBBox().rect.center();
      glEnable(GL_LINE_STIPPLE);
      glBegin(GL_LINES);
      foreach (BBox const & bbox, selectedObj->getBBoxes()) {
         if (bbox.type==BBox::KEYBOX) {
            glVertex2f(prevCenter.x(), prevCenter.y());
            glVertex2f(bbox.rect.center().x(), bbox.rect.center().y());
         }
         prevFrameNumber = bbox.framenumber;
         prevCenter = bbox.rect.center();
      }
      glEnd();
      glDisable(GL_LINE_STIPPLE);

      // set color for foreground lines (2nd pass)
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
      glLineWidth(1.5f);
   }
}

/** Since the frame is saved in a GL texture simply a view filling quad with the
  * said texture is rendered.
  * \note Since the texture is ensured to have power of two dimensions the
  * texture coordinates don't reach to 1.0 but to \ref texCoords.
  */
void VideoWidget::renderCurrentFrame() const {
   glEnable(GL_TEXTURE_2D);
   glBegin(GL_QUADS);
   glTexCoord2f(0.0f, texCoords.height());
   glVertex2f(0.0f, 1.0f);
   glTexCoord2f(texCoords.width(), texCoords.height());
   glVertex2f(1.0f, 1.0f);
   glTexCoord2f(texCoords.width(), 0.0f);
   glVertex2f(1.0f, 0.0f);
   glTexCoord2f(0.0f, 0.0f);
   glVertex2f(0.0f, 0.0f);
   glEnd();
   glDisable(GL_TEXTURE_2D);
}

/** Simply updates the viewport to the new \a width and \a height.
 */
void VideoWidget::resizeGL(int width, int height) {
   glViewport(0, 0, width, height);
}

/** The width and height are powers of two to support legacy video systems.
 * The Coordinate of the loose edge of the actual image data is saved in \ref
 * texCoords for rendering.
 */
void VideoWidget::resizeTexture() {
   const int widthPOT = getNearestPOT(videoSize.width());
   const int heightPOT = getNearestPOT(videoSize.height());
   texCoords = QSizeF(videoSize.width()/float(widthPOT), videoSize.height()/float(heightPOT));

   glEnable(GL_TEXTURE_2D);
   glDeleteTextures(1, &currentTexture);
   glGenTextures(1, &currentTexture);
   glBindTexture(GL_TEXTURE_2D, currentTexture);
   glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexImage2D(GL_TEXTURE_2D, 0, 3, widthPOT, heightPOT, 0, GL_BGR, GL_UNSIGNED_BYTE, 0);
   glDisable(GL_TEXTURE_2D);
}

/** seek: loads frames to show into texture.
 * cacheSize frames before actual frame are cached to achieve better scrollback performance
 * This is where framecaching is done: it consists of 6 cases:
 * 1) cache is empty -> refill.
 * 2) the frame requested is in the cache -> just show.
 * 3) the frame is consecutively following the one in the cache (happens often)
 -> cache new one, delete smallest one.
 * 4) closer to the start than cache is big -> clear cache and refill from start.
 * 5) from new frame backward towards cache is the stuff..
 * 6) not in cache-> empty cache and refill
 */
void VideoWidget::seek(int frame) {
   cv::Mat cvImage;
   const int nextFrame = capture.get(CV_CAP_PROP_POS_FRAMES);
   const int maxFrames = capture.get(CV_CAP_PROP_FRAME_COUNT);

   if (frame == currentFrame){
      updateGL();
      return;
   }
   if ((frame < 0) || (frame >= maxFrames)) {
      play(false);
      return;
   }

   //calculate memory usage:
   if (!cacheEnabled) {
      if (frame != nextFrame) {
         capture.set(CV_CAP_PROP_POS_FRAMES, frame);
      }

      currentFrame = frame;
      emit currentFrameChanged(currentFrame);

      capture >> cvImage;
      updateTexture(cvImage);

      hitArea = NONE;
      updateData();
      updateGL();
   }else{
      int firstElementIdx = fCache.lowerBound(0).key();
      int lastElementIdx = (fCache.end()-1).key();

      // if cache is empty fill,
      if (fCache.isEmpty()){
         if (frame-cacheSize  < 0){
            capture.set(CV_CAP_PROP_POS_FRAMES, 0);
            for (int i=0; i<cacheSize; i++){
               capture >> cvImage; // todo: refine into single line
               fCache.insert(i,cvImage.clone());
            }
         }else{
            capture.set(CV_CAP_PROP_POS_FRAMES, frame-cacheSize);
            for (int i=frame-cacheSize; i<=frame; i++){
               capture >> cvImage; // todo: refine into single line
               fCache.insert(i,cvImage.clone());
            }
         }
         updateTexture(fCache.value(frame));
         // if cache is hit:
      }else if (    (firstElementIdx <= frame)
                    && (lastElementIdx >= frame)){
         updateTexture(fCache.value(frame));
         // if it's in cache+1 (happens often because +1 frame!):
      }else if (frame == lastElementIdx+1){
         // * load new frame, put it to last position of cache, delete the first in the cache
         if(!(capture.get(CV_CAP_PROP_POS_FRAMES) == frame)){
            capture.set(CV_CAP_PROP_POS_FRAMES, frame);
         }
         capture >> cvImage;
         fCache.insert(frame, cvImage.clone());
         updateTexture(fCache.value(frame));
         fCache.remove(firstElementIdx);
         // if too close to beginning to put everything in frame..
      }else if (frame - cacheSize < 0){
         // * delete cache and refill;
         clearFrameCache();
         seek(frame);
         return;
         // already some frames cached.....
      }else if ((frame-cacheSize > firstElementIdx) &&
                (frame-cacheSize < lastElementIdx)){
         // delete frames from first frame to frame-cacheSize,
         // set counter to last Cache Element+1 and add frames from there up to frame
         if(!(capture.get(CV_CAP_PROP_POS_FRAMES) == lastElementIdx+1)){
            capture.set(CV_CAP_PROP_POS_FRAMES, lastElementIdx+1);
         }
         for (int i=lastElementIdx+1; i<=frame; i++){
            capture >> cvImage;
            fCache.insert(i, cvImage.clone());
         }
         for (int i=firstElementIdx; i<frame-cacheSize; i++){
            fCache.remove(i);   // < remove unneeded beginning of cache
         }
         updateTexture(fCache.value(frame));

         // totally not in cache, also old stuff not..
      }else {
         clearFrameCache();
         seek(frame);
         return;
      }
      //std::cout << "Cachesize: " << fCache.size() << " elements. \n";
      currentFrame=frame;
      emit currentFrameChanged(currentFrame);
      hitArea=NONE;
      updateData();
      updateGL();
   }
}

/** If \ref autoZoom is set the zoom is adjusted, too.
  * @sa void setZoom(qreal newZoom)
  */
void VideoWidget::setAvailableSize(QSize newSize) {
   if (availableSize != newSize) {
      availableSize = newSize;
      if (autoZoom && availableSize.isValid() && videoSize.isValid()) {
         setZoom(qMin(availableSize.width()/float(videoSize.width()),
                      availableSize.height()/float(videoSize.height())));
      }
   }
}

/** Also emits signal \ref zoomChanged(float zoom)
  */
void VideoWidget::setZoom(qreal newZoom) {
   zoom = qBound(0.1, newZoom, 2.0);
   emit zoomChanged(zoom);
   resize(videoSize*zoom);
}

/** Simply calls seek(currentFrame+1)
  * @sa void seek(int frame)
  */
void VideoWidget::showNextFrame() {
   /*
   const int nextFrame = capture.get(CV_CAP_PROP_POS_FRAMES);
   const int maxFrames = capture.get(CV_CAP_PROP_FRAME_COUNT);
   if (nextFrame >= maxFrames) {
      play(false);
      return;
   }
   currentFrame = nextFrame;
   emit currentFrameChanged(currentFrame);

   cv::Mat cvImage;
   capture >> cvImage;
   updateTexture(cvImage);

   hitArea = NONE;
   updateData();
   updateGL();
 */
   seek(currentFrame+1);
}

/** Simply calls seek(currentFrame-1)
  * @sa void seek(int frame)
  */
void VideoWidget::showPreviousFrame() {
   /*
      capture.set(CV_CAP_PROP_POS_FRAMES, currentFrame-1.0);
      showNextFrame();
   */
   seek(currentFrame-1);
}

/** If it's over a handle of the selected box it gets changed to a resize cursor
  * else it's reset to the default cursor.
  */
void VideoWidget::updateCursor() {
   if (selectedBBox) {
      switch (isHit(selectedBBox->rect, hitPos)) {
      case TOPLEFT:
         setCursor(Qt::SizeFDiagCursor);
         break;
      case TOP:
         setCursor(Qt::SizeVerCursor);
         break;
      case TOPRIGHT:
         setCursor(Qt::SizeBDiagCursor);
         break;
      case LEFT:
         setCursor(Qt::SizeHorCursor);
         break;
      case CENTER:
         setCursor(Qt::SizeAllCursor);
         break;
      case RIGHT:
         setCursor(Qt::SizeHorCursor);
         break;
      case BOTTOMLEFT:
         setCursor(Qt::SizeBDiagCursor);
         break;
      case BOTTOM:
         setCursor(Qt::SizeVerCursor);
         break;
      case BOTTOMRIGHT:
         setCursor(Qt::SizeFDiagCursor);
         break;
      default:
         unsetCursor();
         break;
      }
   }
   else {
      unsetCursor();
   }
}

/** This should be done whenever the data changes and the cached pointers could
  * be invalid.
  */
void VideoWidget::updateData() {
   bboxes = data->getBBoxes(currentFrame);
   if (selectedObj) {
      selectedBBox = selectedObj->getBBoxPointer(currentFrame);
   }
   updateGL();
}

/** \note OpenCV uses BGR, OpenGL uses RGB.
  */
void VideoWidget::updateTexture(cv::Mat const & mat) const {
   glEnable(GL_TEXTURE_2D);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mat.cols, mat.rows, GL_BGR, GL_UNSIGNED_BYTE, (GLubyte*)mat.data);
   glDisable(GL_TEXTURE_2D);
}

/** Scroll up: Zoom in - Scroll down: Zoom out
 * \note The zoom factor is saved in \ref zoom and is clamped to [0.1, 2.0]
 */
void VideoWidget::wheelEvent(QWheelEvent * event) {
   if (event->modifiers().testFlag(Qt::ControlModifier)) {
      emit zoomChangedManually(false);
      setZoom(zoom+event->delta()/1200.0);
      event->accept();
   }
   else {
      event->ignore();
   }
}

/** The video also gets resized to fit the available size
  */
void VideoWidget::zoomFit(bool checked) {
   if (autoZoom != checked) {
      autoZoom = checked;
      if (availableSize.isValid() && videoSize.isValid()) {
         setZoom(qMin(availableSize.width()/float(videoSize.width()),
                      availableSize.height()/float(videoSize.height())));
      }
   }
}

void VideoWidget::zoomIn() {
   emit zoomChangedManually(false);
   setZoom(zoom+0.1);
}

void VideoWidget::zoomOut() {
   emit zoomChangedManually(false);
   setZoom(zoom-0.1);
}

void VideoWidget::zoomReset() {
   emit zoomChangedManually(false);
   setZoom(1.0);
}

void VideoWidget::toggleCache(){
   if (cacheEnabled){
      cacheEnabled = false;
      clearFrameCache();
      std::cout << "Cache is now disabled." << std::endl;
   }else{
      cacheEnabled = true;
      clearFrameCache();
      std::cout << "Cache is now enabled." << std::endl;
   }
}

void VideoWidget::setCacheProperties(){
   int size;
   if (videoSize.isValid()){
      size = (int)((float)cacheSize*(float)videoSize.width()*(float)videoSize.height()*3.0/(1024.0*1024.0));
      cacheSizeFactor = (float)(videoSize.width()*videoSize.height());
   }else{
      size=0;
      cacheSizeFactor=0.0;
   }

   QDialog * cacheSettingsWidget = new QDialog();
   QFormLayout * settingsLayout = new QFormLayout(cacheSettingsWidget);
   QSpinBox * cacheSizeBox = new QSpinBox();
   QLabel * sizeInMb =  new QLabel(QString::number(size) + QString(" MB"));
   QLabel * newSizeLabel = new QLabel();
   QPushButton *okBut = new QPushButton(tr("Ok"));
   QPushButton *cancelBut = new QPushButton(tr("Cancel"));

   cacheSettingsWidget->setWindowTitle(tr("Set Cache Size properties"));

   cacheSizeBox->setMinimum(5);
   cacheSizeBox->setValue(cacheSize);

   connect(cacheSizeBox, SIGNAL(valueChanged(int)), this, SLOT(setCacheSizeText(int)));
   connect(this, SIGNAL(cacheSizeChanged(QString)), newSizeLabel, SLOT(setText(QString)));
   connect(cancelBut, SIGNAL(clicked()), cacheSettingsWidget, SLOT(reject()));
   connect(okBut, SIGNAL(clicked()), cacheSettingsWidget, SLOT(accept()));

   settingsLayout->addRow("Current Cache size: ", sizeInMb);
   settingsLayout->addRow("Cache size in Frames:", cacheSizeBox);
   settingsLayout->addRow("New cache size:", newSizeLabel);
   settingsLayout->addRow(okBut, cancelBut);

   setCacheSizeText(cacheSize);

   if (cacheSettingsWidget->exec() == QDialog::Accepted){
      setCacheSize(cacheSizeBox->value());
   }
   cacheSettingsWidget->deleteLater();
}

/** The cahce also gets cleared
  */
void VideoWidget::setCacheSize(int newSize){
   cacheSize = newSize;
   clearFrameCache();
}

void VideoWidget::setCacheSizeText(int size){
   emit cacheSizeChanged(QString::number((int)(((float)size *cacheSizeFactor *3.0)/(1024.0*1024.0)))+ QString(" MB"));
}

void VideoWidget::clearFrameCache(){
   fCache.clear();
}

void VideoWidget::toggleCenterlines(){
   showCenterLines = !showCenterLines;
   updateGL();
}
