#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QtOpenGL/QGLWidget>
#include <opencv2/highgui/highgui.hpp>
#include <QtGui/QLabel>
#include "types.h"

class DataWidget;

/// Class managing the video data and doing all the rendering.
/** The GL widget can load video files using OpenCV and can render it and the
  * corresponding bounding boxes from the TrackingdataWidget via OpenGL. It also
  * allows direct mouse interaction with the boxes.
  */
class VideoWidget : public QGLWidget {

   Q_OBJECT

   /// Denotes the hitarea of a bounding box
   /** It is used to return which area was hit by a mouse event.
     */
   enum Hitarea {
      NONE=0,     ///< Box wasn't hit
      TOPLEFT,    ///< Top left handle was hit
      TOP,        ///< Top handle was hit
      TOPRIGHT,   ///< Top right handle was hit
      LEFT,       ///< Left handle was hit
      CENTER,     ///< Area of the box which is not a handle was hit
      RIGHT,      ///< Right handle was hit
      BOTTOMLEFT, ///< Bottom left handle was hit
      BOTTOM,     ///< Bottom handle was hit
      BOTTOMRIGHT ///< Bottom right handle was hit
   };

public:
   /// Ctor which takes a pointer to a DataWidget
   explicit VideoWidget(DataWidget * data, QWidget * parent = 0);
   /// Returns the Framerate of the current video
   double getFramerate();

signals:
   /// Gets emitted when the video is started/paused
   /** This signal is used to keep the play/pause button on the GUI in sync,
     * where \a play holds whether the video is now playing or paused.
     * @sa void MainWindow::togglePlayPause(bool play)
     */
   void playToggled(bool play);
   /// Gets emitted each time a new frame is shown
   /** the new frames number is submitted in \a n.
     * @sa void seek(int frame)
     * @sa DataWidget::setCurrentFrame(int framenumber)
     * @sa DataWidget::currentFrameChanged(int framenumber)
     */
   void currentFrameChanged(int n);
   /// Gets emitted when an other box gets selected and submits its objects \a ID.
   /** This is part of the mechanism to keep the selection in different views
     * consistent.
     * @sa void changeSelection(int id)
     * @sa void DataWidget::setSelectedObject(int id)
     * @sa void DataWidget::selectedObjectChanged(int id)
     */
   void selectionChanged(int id);
   /// Gets emitted whenever the zoomfactor \ref zoom changes
   /** This is used to keep the zoomLabel of the MainWindow in sync.
     */
   void zoomChanged(float zoom);
   /// Gets emitted whenever the zoom is changed actively by the user
   /** This is used to deactivate autozoom on user interaction
     */
   void zoomChangedManually(bool manually);
   /// Emitted whenever a new video is opened
   /** This is used to keep the seek sliders range in sync.
     */
   void maxFramesChanged(int n);
   /// Emitted whenever the size of the framecache changes
   void cacheSizeChanged(QString newCacheSizeText);
   /// Emitted with false whenever a initiated box creation gets aborted.
   /** This notation is used so it can connect directly to a setChecked(bool)
     * slot of a QAction.
     */
   void boxCreationStarted(bool started);
   /// Emitted whenever the actions of the MainWindow should update
   void updateActions();

public slots:
   /// Opens a video file using OpenCV
   void openVideoFile();
   /// Opens the video file with the given \a openFilename using openCV
   void openRequest(QString openFilename);
   /// Shows the next frame of the video
   void showNextFrame();
   /// Shows the previous frame of the video
   void showPreviousFrame();
   /// Toggles Play/Pause
   void play(bool play = true);
   /// shows a frame specified by its framenumber \a frame
   void seek(int frame);
   /// Initiates the creation of a new bounding box
   void createBBox();
   /// Initiates the creation of a new \ref BBox::KEYBOX "key" bounding box
   void createKeyBBox();
   /// Zooms in a bit
   void zoomIn();
   /// Zooms out a bit
   void zoomOut();
   /// Sets \ref autoZoom to \a checked
   void zoomFit(bool checked);
   /// Resets the zoomfactor
   void zoomReset();
   /// Changes the \ref selectedObj "selected object" to the object with the specified \a ID.
   void changeSelection(int id);
   /// Rerequests the cached data from the DataWidget
   void updateData();
   /// Sets the \ref availableSize to \a newSize
   void setAvailableSize(QSize newSize);
   /// Toggles frame caching.
   void toggleCache();
   /// Shows a dialog to edit the cache settings.
   void setCacheProperties();
   /// Sets the frame cahce size to \a newSize
   void setCacheSize(int newSize);
   /// Updates the frame cache size label of the properties dialog.
   void setCacheSizeText(int size);
   /// Clears the frame cache.
   void clearFrameCache();
   /// Toggles the visibilit of the centerlines used to trace objects
   void toggleCenterlines();

protected:
   /// Mouse wheel event handler
   void wheelEvent(QWheelEvent * event);
   /// Mouse move event handler
   void mouseMoveEvent(QMouseEvent * event);
   /// Mouse button press event handler
   void mousePressEvent(QMouseEvent * event);
   /// Mouse button release event handler
   void mouseReleaseEvent(QMouseEvent * event);

private:
   bool autoZoom;             ///< Indicates whether or not the widget should adjust its size to the available space.
   bool showCenterLines;      ///< Indicates whether or not the centerline of the selected object should be shown
   bool createBoxFlag;        ///< Indicates that a bounding box should be created
   bool createKeyboxFlag;     ///< Indicates that a key bounding box should be created
   QSize videoSize;           ///< The video's resolution
   QSize availableSize;       ///< The available size for the widget
   qreal zoom;                ///< The current zoomfactor
   QSizeF texCoords;          ///< The boundary of the texture coordinates to render the video data
   GLuint currentTexture;     ///< OpenGL name of the texture holding the current frame
   int currentFrame;          ///< The number of the currently shown frame
   Object * selectedObj;      ///< The currently selected object
   BBox * selectedBBox;       ///< The currently selected bounding box
   Hitarea hitArea;           ///< The area hitten by a click on a bounding box
   QPoint hitPos;             ///< The point hitten by the mouse on the video
   QList<BBox> bboxes;        ///< List of currently visible bounding boxes
   cv::VideoCapture capture;  ///< The OpenCV capture holding the video data
   DataWidget * data;         ///< Pointer to the tracking data
   QTimer * timer;            ///< Timer for video playback
   QMap<int, cv::Mat> fCache; ///< FrameCache for faster scrollback!
   bool cacheEnabled;         ///< Indicates whether or not the framecaching should be active
   int cacheSize;             ///< The cache size
   int cacheSizeFactor;       ///< The cache size factor

   /// Initialization after context creation
   void initializeGL();
   /// Rendering happens here
   void paintGL();
   /// Resize event handler
   void resizeGL(int width, int height);
   /// Renders the current video frame
   void renderCurrentFrame() const;
   /// Renders a bounding box
   void renderBBox(BBox const & bbox, bool active = false) const;
   /// Renders the centerline for the active object
   void renderCenterline() const;
   /// Renders the currently selected object
   void renderSelectedObject() const;
   /// Recreates the frame texture with the specified size
   void resizeTexture();
   /// Uploads the frame in \a mat to the frame texture
   void updateTexture(cv::Mat const & mat) const;
   /// Updates the cursor according to its context
   void updateCursor();
   /// Determines which Hitarea (handle) of the \a rect was hitten by the cursors \a pos
   Hitarea isHit(QRect const & rect, QPoint const & pos) const;
   /// Sets the zoom level to \a newZoom
   void setZoom(qreal newZoom);
};

#endif // VIDEOWIDGET_H
