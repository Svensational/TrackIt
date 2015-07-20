#include "datawidget.h"
#include <QtCore/QFile>
#include <QtCore/QStack>
#include <QtCore/QTextStream>
#include <QtGui/QBoxLayout>
#include <QtGui/QButtonGroup>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QInputDialog>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QProgressDialog>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QTableView>
#include <QtGui/QToolButton>
#include <QtXml/QDomDocument>
#include "category.h"
#include "object.h"
#include "bboxdelegate.h"
#include "idcounter.h"

/** Also creates a default category and object for a swifter start.
  * @sa <a href="http://qt-project.org/doc/qt-4.8/qtabwidget.html#QTabWidget">
  *     QTabWidget::QTabWidget(QWidget * parent = 0)</a>
  */
DataWidget::DataWidget(QWidget * parent) :
   QTabWidget(parent), zoom(1), currentFrameNr(-1), selectedObjectID(-1),
   filename(QString()), videofileInfo(VideofileInfo()),
   closeBtnGroup(new QButtonGroup(this)), editBtnGroup(new QButtonGroup(this))
{
   setContextMenuPolicy(Qt::CustomContextMenu);

   createNewCategoryTab();
   createCornerWidget();

   connect(editBtnGroup, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(editCategory(QAbstractButton *)));
   connect(closeBtnGroup, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(deleteCategory(QAbstractButton *)));
   connect(this, SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabChanged(int)));

   newObject();
}

DataWidget::~DataWidget() {
   qDeleteAll(categories);
   categories.clear();
}

/** The category is appended to the internal list of categories. Also a new
  * QTableView with the category as model is created and added as a new tab.
  * @note Category derives from QAbstractListModel to make this simple
  * connection possible.
  */
void DataWidget::addCategory(Category * cat) {
   // add category
   categories << cat;
   cat->setColumnCount(videofileInfo.framecount);

   // add tab
   QTableView * tableView = new QTableView();
   BBoxDelegate * delegate = new BBoxDelegate();
   tableView->verticalHeader()->setDefaultSectionSize(20);
   tableView->horizontalHeader()->setDefaultSectionSize(zoom);
   tableView->horizontalHeader()->setMinimumSectionSize(1);
   tableView->horizontalHeader()->hide();
   tableView->setShowGrid(false);

   tableView->setItemDelegate(delegate);
   tableView->setModel(cat);
   tableView->setSelectionMode(QAbstractItemView::ContiguousSelection);
   connect(tableView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
           this, SLOT(selectionChanged(QModelIndex, QModelIndex)));

   int index = insertTab(count()-1, tableView, QIcon(":/icons/category-16"), cat->getName());

   // add tab buttons
   QWidget * btnWidget = new QWidget();
   QHBoxLayout * layout = new QHBoxLayout();
   layout->setSpacing(0);
   layout->setContentsMargins(0, 0, 0, 0);

   QToolButton * editBtn = new QToolButton();
   QIcon icon = QIcon(inactivePixmap(":/icons/edit-12"));
   icon.addPixmap(QPixmap(":/icons/edit-12"), QIcon::Active);
   editBtn->setIcon(icon);
   editBtn->setAutoRaise(true);
   editBtn->setIconSize(QSize(12, 12));
   editBtn->setToolTip(tr("Rename category"));
   layout->addWidget(editBtn);
   editBtnGroup->addButton(editBtn);

   QToolButton * closeBtn = new QToolButton();
   icon = QIcon(inactivePixmap(":/icons/delete-12"));
   icon.addPixmap(QPixmap(":/icons/delete-12"), QIcon::Active);
   closeBtn->setIcon(icon);
   closeBtn->setAutoRaise(true);
   closeBtn->setIconSize(QSize(12, 12));
   closeBtn->setToolTip(tr("Delete category"));
   layout->addWidget(closeBtn);
   closeBtnGroup->addButton(closeBtn);

   btnWidget->setLayout(layout);
   tabBar()->setTabButton(index, QTabBar::RightSide, btnWidget);
   setCurrentIndex(index);

   emit categoryCountChanged(count()-1);
}

/** @sa void DataWidget::addCategory(Category * cat)
  */
Category * DataWidget::addCategory(QString const & name) {
   Category * cat = new Category(name);
   addCategory(cat);
   return cat;
}

/** If the specified category doesn't exist it gets created before inserting.
  * @sa Category & Category::operator <<(Object * object)
  */
void DataWidget::addObject(Object * object, QString const & catName) {
   bool exists = false;
   int i = 0;
   Category * cat = NULL;
   while (!exists && i<categories.size()) {
      cat = categories.at(i++);
      exists = (cat->getName() == catName);
   }
   if (!exists) {
      cat = addCategory(catName);
   }
   cat->addObject(object);
}

/** The section size of all header views gets set to the new zoomlevel and the
  * current view gets scrolled to keep the current selection centered.
  */
void DataWidget::changeZoom(int newZoom) {
   if (zoom != newZoom) {
      zoom = newZoom;
      QHeaderView * headerView;
      for (int i=0; i<count()-1; ++i) {
         headerView = static_cast<QTableView *>(widget(i))->horizontalHeader();
         headerView->setDefaultSectionSize(zoom);
         headerView->resizeSections(QHeaderView::Interactive);
      }
   }
   QTableView * tableView = static_cast<QTableView *>(currentWidget());
   tableView->scrollTo(tableView->currentIndex(), QAbstractItemView::PositionAtCenter);
}

/** The user gets warned and asked to confirm the process.
  * @sa void clearDataImmediate()
  */
void DataWidget::clearData() {
   QMessageBox msgBox;
   msgBox.setIconPixmap(QPixmap(":/icons/clear-32"));
   msgBox.setText(tr("Do you really want to clear everything?"));
   msgBox.setInformativeText("All categories, objects and bounding boxes will be lost and the id counter will be reset.");
   msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
   if (msgBox.exec() == QMessageBox::Yes) {
      clearDataImmediate();
   }
}

/** This is called before new data is imported. Signal dataDecreased()
  * gets emitted afterwards.
  * @sa void clearData()
  */
void DataWidget::clearDataImmediate() {
   emit selectedObjectChanged(-1);
   QWidget * w;
   while (count()>1) {
      w = widget(0);
      removeTab(0);
      delete w;
   }
   qDeleteAll(categories);
   categories.clear();
   idCounter->reset();
   emit dataDecreased();
   emit categoryCountChanged(0);
}

/** The corner widget contains a button for the clear action as well as the
  * spinbox to adjust the zoomlevel.
  */
void DataWidget::createCornerWidget() {
   QWidget * cornerWidget = new QWidget();
   QHBoxLayout * layout = new QHBoxLayout();
   layout->setSpacing(0);
   layout->setContentsMargins(0, 0, 0, 0);

   QToolButton * clearBtn = new QToolButton();
   //QIcon icon = QIcon(grayscale(":/icons/clear-16"));
   //icon.addPixmap(QPixmap(":/icons/clear-16"), QIcon::Active);
   clearBtn->setIcon(QIcon(":/icons/clear-16"));
   clearBtn->setIconSize(QSize(16, 16));
   clearBtn->setAutoRaise(true);
   clearBtn->setToolTip(tr("Clear all data, reset IDs"));
   connect(clearBtn, SIGNAL(clicked()), this, SLOT(clearData()));
   layout->addWidget(clearBtn);

   layout->addSpacing(5);

   QLabel * glassLbl = new QLabel();
   glassLbl->setPixmap(QPixmap(":/icons/zoom-16"));
   layout->addWidget(glassLbl);

   QSpinBox * zoomSpin = new QSpinBox();
   zoomSpin->setRange(1, 16);
   zoomSpin->setValue(1);
   zoomSpin->setFixedWidth(35);
   connect(zoomSpin, SIGNAL(valueChanged(int)), this, SLOT(changeZoom(int)));
   layout->addWidget(zoomSpin);

   cornerWidget->setLayout(layout);
   setCornerWidget(cornerWidget);
}

/** The tab has a special '+' icon and contains only a button to create a new
  * tab manually (only used when it's the only tab)
  */
void DataWidget::createNewCategoryTab() {
   QWidget * widget = new QWidget();
   QGridLayout * layout = new QGridLayout();

   QPushButton * button = new QPushButton(QIcon(":/icons/add-16"), tr("Add new category"));
   connect(button, SIGNAL(clicked()), this, SLOT(newCategory()));
   layout->addWidget(button, 1, 1);

   layout->setColumnStretch(0, 1);
   layout->setColumnStretch(2, 1);
   layout->setRowStretch(0, 1);
   layout->setRowStretch(2, 1);
   widget->setLayout(layout);
   setTabToolTip(addTab(widget, QIcon(":/icons/add-16"), ""), tr("Add a new category"));
}

/** The user gets asked for confirmation and then deletion orders get sent to
  * the current category
  * @sa void Category::deleteBBoxes(QModelIndexList const & selection)
  */
void DataWidget::deleteBBox() {
   const int i = currentIndex();
   if (i>=0) {
      QModelIndexList selected = static_cast<QTableView *>(widget(i))->selectionModel()->selectedIndexes();
      if (!selected.isEmpty()) {
         QMessageBox msgBox;
         msgBox.setIconPixmap(QPixmap(":/icons/deletebox-32"));
         msgBox.setText(tr("Do you really want to delete the selected bounding boxes?"));
         msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
         if (selected.size()==1 || msgBox.exec()==QMessageBox::Yes) {
            //emit selectedObjectChanged(-1);
            categories.at(i)->deleteBBoxes(selected);
            emit dataDecreased();
         }
      }
   }
}

/** The current index gets determined and the corresponding function gets called
  * @sa void deleteCategory(int index)
  * @sa void deleteCategory(QAbstractButton * button)
  */
void DataWidget::deleteCategory() {
   deleteCategory(currentIndex());
}

/** The coresponding category gets determined via the closeBtnGroup.
  * @sa void deleteCategory(int index)
  * @sa void deleteCategory()
  */
void DataWidget::deleteCategory(QAbstractButton * button) {
   deleteCategory(closeBtnGroup->buttons().indexOf(button));
}

/** The user gets asked for confirmation and the corresponding signals get emitted
  * @sa void deleteCategory()
  * @sa void deleteCategory(QAbstractButton * button)
  */
void DataWidget::deleteCategory(int index) {
   if (index>=0) {
      QMessageBox msgBox;
      msgBox.setIconPixmap(QPixmap(":/icons/deletecategory-32"));
      msgBox.setText(tr("Do you really want to delete the category ")+categories.at(index)->getName()+"?");
      msgBox.setInformativeText("All contained objects will be lost, too.");
      msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
      if (categories.at(index)->isEmpty() || msgBox.exec() == QMessageBox::Yes) {
         if (index==currentIndex() && index==count()-2) {
            // if the last valid tab was selected the "add new cat." tab would be selected afterwards
            setCurrentIndex(index-1);
         }
         emit selectedObjectChanged(-1);
         QWidget * tableView = widget(index);
         removeTab(index);
         delete tableView;
         delete categories.takeAt(index);
         emit dataDecreased();
         emit categoryCountChanged(count()-1);
      }
   }
}

/** Actually the selection gets converted to a rownumber and the corresponding
  * function from the currently shown category is called.
  * @sa void Category::deleteObjectAt(int row)
  */
void DataWidget::deleteObject() {
   const QList<int> rows = getSelectedRows();
   if (rows.size() > 0) {
      Category * const category = categories.at(currentIndex());
      // check whether all objects are empty or not
      bool allEmpty = true;
      foreach (int row, rows) {
         if (!category->getObjects().at(row)->isEmpty()) {
            allEmpty = false;
         }
      }
      // create dialog
      QMessageBox msgBox;
      msgBox.setIconPixmap(QPixmap(":/icons/deleteobject-32"));
      if (rows.size() == 1) {
         msgBox.setText(tr("Do you really want to delete the selected object?"));
      }
      else {
         msgBox.setText(tr("Do you really want to delete all (%1) selected objects?").arg(rows.size()));
      }
      msgBox.setInformativeText("All contained bounding boxes will be lost, too.");
      msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
      // ask user for permission and delete
      if (allEmpty || msgBox.exec() == QMessageBox::Yes) {
         emit selectedObjectChanged(-1);
         foreach (int row, rows) {
            category->deleteObjectAt(row);
         }
         emit dataDecreased();
      }
      // select last object after deleted ones
      int newSelection = qMin(category->getObjects().size()-1, rows.last());
      setSelection(currentIndex(), newSelection, currentFrameNr);
   }
}

/** Determines the current index and calls the appropriate function
  * @sa void editCategory(int index)
  * @sa void editCategory(QAbstractButton * button)
  */
void DataWidget::editCategory() {
   editCategory(currentIndex());
}

/** Determines the index of the button using the editBtnGroup and calls the
  * appropriate function
  * @sa void editCategory(int index)
  * @sa void editCategory()
  */
void DataWidget::editCategory(QAbstractButton * button) {
   editCategory(editBtnGroup->buttons().indexOf(button));
}

/** A QInputDialog is shown to get the new category name from the user.
  * @sa void editCategory()
  * @sa void editCategory(QAbstractButton * button)
  */
void DataWidget::editCategory(int index) {
   if (index < 0) return;

   bool ok;
   QString name = QInputDialog::getText(this, tr("Rename"),
                                        tr("New name of the category:"),
                                        QLineEdit::Normal,
                                        tabText(index), &ok);
   if (ok && !name.isEmpty()) {
      categories.at(index)->setName(name);
      setTabText(index, name);
   }
}

/** The selection of the new category happens via a dropdown containing all
  * existing categories.
  */
void DataWidget::editObject() {
   const QList<int> rows = getSelectedRows();
   if (rows.size() > 0) {
      // get existing category names
      QStringList items;
      foreach (Category * cat, categories) {
         items << cat->getName();
      }
      // get target category
      bool ok;
      QString item = QInputDialog::getItem(this,
                                           tr("Change category"),
                                           tr("New category:"),
                                           items, currentIndex(), false, &ok);
      if (ok && !item.isEmpty()) {
         Category * const category = categories.at(currentIndex());
         QStack<Object *> objects;
         emit selectedObjectChanged(-1);
         // remove objects from current category (reverse order)
         foreach (int row, rows) {
            objects.push(category->takeObjectAt(row));
         }
         // add objects to new category (reverse reverse order)
         while (!objects.isEmpty()) {
            addObject(objects.pop(), item);
         }
         emit dataDecreased();
      }
   }
}

/** Asks the user for a filename and type, creates a file and calls the
  * appropriate export function.
  * @sa void exportBBFile(QFile & file)
  * @sa void exportViperFile(QFile & file)
  */
void DataWidget::exportFile() {
   QString xmlFilter = tr("ViPER files (*.xml)");
   QString bbFilter = tr("BB files (*.bb)");
   QString selectedFilter = xmlFilter;

   // get filename from user
   QString filename = QFileDialog::getSaveFileName(this,
                                                   tr("Export data"),
                                                   QString(),
                                                   xmlFilter + ";;" + bbFilter,
                                                   &selectedFilter);
   if (filename.isEmpty()) {
      return;
   }
   QDir::setCurrent(filename.section('/', 0, -2));

   // mangle filenames for correct suffixes
   if (selectedFilter == xmlFilter) {
      if (!filename.endsWith(".xml", Qt::CaseInsensitive)) {
         if (QMessageBox::question(this, "No file extension",
                                   "Do you want to append \".xml\" to the filename?",
                                   QMessageBox::Yes|QMessageBox::No)
             == QMessageBox::Yes) {
            filename.append(".xml");
         }
      }
   }
   else if (selectedFilter == bbFilter) {
      if(!filename.endsWith(".bb", Qt::CaseInsensitive)) {
         if (QMessageBox::question(this, "No file extension",
                                   "Do you want to append \".bb\" to the filename?",
                                   QMessageBox::Yes|QMessageBox::No)
             == QMessageBox::Yes) {
            filename.append(".bb");
         }
      }
   }
   // Save file.
   QFile file(filename);
   if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
      QMessageBox::warning(this, tr("Unable to open file"), tr("Unable to create/open file\n\"")
                                                            +filename.section('/', -1)
                                                            +tr("\"\nfor writing!"));
      return;
   }
   // Determine filetype according to the selected filter.
   if (selectedFilter == xmlFilter) {
      exportViperFile(file);
   }
   else if (selectedFilter == bbFilter) {
      exportBBFile(file);
   }
   else {
      QMessageBox::warning(this, tr("Unknown file type"), tr("The requested type of the file\n\"")
                                                          +filename.section('/', -1)
                                                          +tr("\"\nsomehow couldn't be determined!"));
   }
   file.close();
}

/**
  * @note Since the BB file format doesn't support categories, interpolated boxes
  *       and gaps between frames this information will be lost.
  * @sa void exportFile()
  * @sa void exportViperFile(QFile & file)
  */
void DataWidget::exportBBFile(QFile & file) {
   QTextStream out (&file);
   out << videofileInfo.framecount << endl;
// -- datenstruktur überdenken. evtl erst alles mal collecten und nach framenummer sortieren..
// uff, das wird hässlich. liste von BBox-Listen erstellen. Objekte aufteilen
   int currentFrame;
   Object * currentObj;
   QMultiMap< int, Object*> frameList;
   // get all bboxes from all objects in all categories. If objects contain non-consecutive-frames, split them into
   // different objects (because else bb file format does not support)
   //std::cout << "info->framecount ist: " << videofileInfo->framecount << std::endl;
   foreach(Category const * const category, categories) {
      foreach (Object const * const object, category->getObjects()) {
          if (!(object->isEmpty())){
             currentObj = new Object();
             currentFrame = object->getBBoxes().begin().value().framenumber; // point to first bbox in qmap - has to be the one with the lowest key, a.k.a. framenumber
             frameList.insert(currentFrame, currentObj);
             // add BBoxes, if non-consecutive, split objects
             foreach (BBox const currentBBox, object->getBBoxes()) {
                // generate Bboxes from virtual frames and add them
                if ((currentBBox.framenumber != currentFrame ) && (currentBBox.type == BBox::KEYBOX)){
                   BBox lastBBox = (object->getBBoxes().lowerBound(currentBBox.framenumber)-1).value();
                   currentObj->addBBox(lastBBox);
                   for (int i=lastBBox.framenumber+1; i<currentBBox.framenumber; i++){
                     currentObj->addBBox( interpolate(i,lastBBox,currentBBox) );
                   }
                   currentObj->addBBox(currentBBox);
                   currentFrame=currentBBox.framenumber;
                // or split into new objects
                }else if (currentBBox.framenumber != currentFrame ){
                   currentFrame = currentBBox.framenumber;
                   currentObj = new Object();
                   currentObj->addBBox(currentBBox);
                   frameList.insert(currentFrame, currentObj);
                // or just add
                }else{
                   currentObj->addBBox(currentBBox);
                }
                currentFrame++;
             }
          }
      }
   }

   // now print to file:
   QList<Object*> objects;
   out.setRealNumberNotation(QTextStream::FixedNotation);
   // for all frames
   for(int frameNo = 0; frameNo < videofileInfo.framecount; frameNo ++){
      objects = frameList.values(frameNo );
      //std::cout << "# of objs in frame " << frameNo << ": " << objects.size() << std::endl;
      out << frameNo  << endl;
      out << objects.size() << endl;
      // alle objekte des frames
      QListIterator<Object *> i(objects);
      i.toBack();
      while (i.hasPrevious()){
         //std::cout << "object " << i.peekPrevious() << std::endl;
         out << (i.peekPrevious()->getBBoxes()).size() << endl;
         //fuer jedes objekt alle bboxen:
         // ohne erst in int parsen zu müssen wär's schneller?
         foreach (BBox const currentBox, i.previous()->getBBoxes()){
            out << double(currentBox.rect.left()) <<";";
            out << double(currentBox.rect.top())  <<";";
            out << double(currentBox.rect.width())<<";";
            out << double(currentBox.rect.height())<<endl;
         }
      }
   }
}

/** @note Since the ViPER file format doesnt support interpolated boxes they
  *       will be transformed to single boxes.
  * @note The official definition of the ViPER file format can be viewed here:
  * <a href="http://viper-toolkit.sourceforge.net/docs/file/">ViPER XML</a>
  * @sa void exportFile()
  * @sa void exportBBFile(QFile & file)
  */
void DataWidget::exportViperFile(QFile & file) {
   int progressMax = 1;
   foreach(Category const * const category, categories) {
      progressMax += category->rowCount();
   }
   QProgressDialog progress(tr("Exporting ViPER file..."), tr("Abort"), 0, progressMax, this);
   progress.setWindowModality(Qt::WindowModal);
   progress.show();
   progress.setValue(0);

   QDomDocument doc;
   QDomProcessingInstruction header = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
   doc.appendChild(header);

   QDomElement rootDE = doc.createElement("viper");
   rootDE.setAttribute("xmlns", "http://lamp.cfar.umd.edu/viper");
   rootDE.setAttribute("xmlns:data", "http://lamp.cfar.umd.edu/viperdata");
   doc.appendChild(rootDE);

   QDomElement configDE = doc.createElement("config");
   rootDE.appendChild(configDE);

   QDomElement descriptorDE = doc.createElement("descriptor");
   descriptorDE.setAttribute("name", "Information");
   descriptorDE.setAttribute("type", "FILE");
   configDE.appendChild(descriptorDE);

   QDomElement attributeDE = doc.createElement("attribute");
   attributeDE.setAttribute("dynamic", "false");
   attributeDE.setAttribute("name", "NUMFRAMES");
   attributeDE.setAttribute("type", "dvalue");
   descriptorDE.appendChild(attributeDE);

   attributeDE = doc.createElement("attribute");
   attributeDE.setAttribute("dynamic", "false");
   attributeDE.setAttribute("name", "H-FRAME-SIZE");
   attributeDE.setAttribute("type", "dvalue");
   descriptorDE.appendChild(attributeDE);

   attributeDE = doc.createElement("attribute");
   attributeDE.setAttribute("dynamic", "false");
   attributeDE.setAttribute("name", "V-FRAME-SIZE");
   attributeDE.setAttribute("type", "dvalue");
   descriptorDE.appendChild(attributeDE);

   foreach(Category const * const category, categories) {
      descriptorDE = doc.createElement("descriptor");
      descriptorDE.setAttribute("name", category->getName());
      descriptorDE.setAttribute("type", "OBJECT");
      configDE.appendChild(descriptorDE);

      attributeDE = doc.createElement("attribute");
      attributeDE.setAttribute("dynamic", "true");
      attributeDE.setAttribute("name", "BoundingBox");
      attributeDE.setAttribute("type", "bbox");
      descriptorDE.appendChild(attributeDE);
   }

   // write the data section
   QDomElement dataDE = doc.createElement("data");
   rootDE.appendChild(dataDE);

   QDomElement sourcefileDE = doc.createElement("sourcefile");
   sourcefileDE.setAttribute("filename", videofileInfo.filename);
   dataDE.appendChild(sourcefileDE);

   QDomElement fileDE = doc.createElement("file");
   fileDE.setAttribute("id", 0);
   fileDE.setAttribute("name", "Information");
   sourcefileDE.appendChild(fileDE);

   attributeDE = doc.createElement("attribute");
   attributeDE.setAttribute("name", "NUMFRAMES");
   fileDE.appendChild(attributeDE);
   QDomElement datavalueDE = doc.createElement("data:dvalue");
   datavalueDE.setAttribute("value", videofileInfo.framecount);
   attributeDE.appendChild(datavalueDE);

   attributeDE = doc.createElement("attribute");
   attributeDE.setAttribute("name", "H-FRAME-SIZE");
   fileDE.appendChild(attributeDE);
   datavalueDE = doc.createElement("data:dvalue");
   datavalueDE.setAttribute("value", videofileInfo.size.width());
   attributeDE.appendChild(datavalueDE);

   attributeDE = doc.createElement("attribute");
   attributeDE.setAttribute("name", "V-FRAME-SIZE");
   fileDE.appendChild(attributeDE);
   datavalueDE = doc.createElement("data:dvalue");
   datavalueDE.setAttribute("value", videofileInfo.size.height());
   attributeDE.appendChild(datavalueDE);

   int counter = 1;

   foreach(Category const * const category, categories) {
      foreach (Object const * const object, category->getObjects()) {
         sourcefileDE.appendChild(object->toViperNode(doc, category->getName()));
         progress.setValue(counter++);
         if (progress.wasCanceled()) {
            return;
         }
      }
   }

   QTextStream out(&file);
   out << doc.toString(4);
   progress.setValue(progressMax);
}

/** Internally all categories get asked for their matching bounding boxes, which
  * in turn ask all their objects for their matching bounding boxes. So this
  * kinda ripples through until the actual boxes are reached.
  * @note Since the returned list contains copies of the boxes altering them
  * will not affect the original data.
  * @sa QList<BBox> Category::getBBoxes(int framenumber) const
  * @sa BBox Object::getBBox(int framenumber) const
  */
QList<BBox> DataWidget::getBBoxes(int framenumber) const {
   QList<BBox> bboxes;
   foreach (Category const * const category, categories) {
      bboxes << category->getBBoxes(framenumber);
   }
   return bboxes;
}

/** This function is used to keep selection dependent actions in sync
  */
BBox::Type DataWidget::getCurrentBBoxType() const {
   Cell cell = getSelection();
   return categories.at(cell.index)->getObjects().at(cell.row)->getBBox(cell.column).type;
}

/** If no such object exists a NULL pointer is returned instead.
  */
Object * DataWidget::getObject(int id) const {
   Object * object = NULL;
   int i=0;
   while (!object && i<categories.size()) {
      object = categories.at(i++)->getObject(id);
   }
   return object;
}

/** @note The list of rownumbers doesn't contain duplicates and is sorted in descending order
  */
QList<int> DataWidget::getSelectedRows() const {
   QList<int> rows;
   if (currentIndex() >= 0) {
      const QModelIndexList selected = static_cast<QTableView *>(currentWidget())->selectionModel()->selectedIndexes();
      if (!selected.isEmpty()) {
         // extract the row numbers, omit duplicates
         foreach (QModelIndex const & index, selected) {
            if (!rows.contains(index.row())) {
               rows << index.row();
            }
         }
         // reverse sort
         qSort(rows.begin(), rows.end(), qGreater<int>());
      }
   }
   return rows;
}

/** The selection consists of the corresponding tab index and the table row and
  * column.
  */
DataWidget::Cell DataWidget::getSelection() const {
   return Cell(currentIndex(), static_cast<QTableView *>(currentWidget())->currentIndex());
}

/** The corresponding filename gets asked from the user via a QFileDialog and
  * the file is opened using the apropriate function.
  * @sa void importBBFile(QFile & file)
  * @sa void importViperFile(QFile & file)
  */
void DataWidget::importFile() {
   QString selectedFilter = tr("Supported files (*.xml *.bb)");
   QString xmlFilter = tr("ViPER files (*.xml)");
   QString bbFilter = tr("BB files (*.bb)");
   // Get a filename from the user
   QString openFilename = QFileDialog::getOpenFileName(this,
                                                       tr("Import data file"),
                                                       QString(),
                                                       selectedFilter+";;"+xmlFilter+";;"+bbFilter+tr(";;All files (*)"),
                                                       &selectedFilter);
   if (openFilename.isEmpty()) {
      return;
   }

   QDir::setCurrent(openFilename.section('/', 0, -2));

   //Open the file
   QFile file(openFilename);
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QMessageBox::warning(this, tr("Unable to open file"),
                           tr("The file\n\"")
                           +openFilename.section('/', -1)
                           +tr("\"\ncouldn't be opened!"));
      return;
   }

   if (selectedFilter == xmlFilter || openFilename.section('.', -1).toLower() == "xml") {
      // viper file
      importViperFile(file);
   }
   else if (selectedFilter == bbFilter || openFilename.section('.', -1).toLower() == "bb") {
      // bb file
      importBBFile(file);
   }
   else {
      // try to determine the filetype
      char buf;
      file.peek(&buf, sizeof(buf));
      if (buf == '<') {
         // probably a viper file
         importViperFile(file);
      }
      else if (buf>='0' && buf<='9') {
         // probably a bb file
         importBBFile(file);
      }
      else {
         QMessageBox::warning(this, tr("Unknown file type"),
                              tr("The type of the file\n\"")
                              +openFilename.section('/', -1)
                              +tr("\"\ncouldn't be determined!"));
      }
   }

   file.close();
   // reset filename
   filename = QString();
   updateFramecount();
}

/** If the reading or parsing fails at any stage before tracking data could be
  * read the function simply aborts.
  * @sa void importFile()
  * @sa void importViperFile(QFile & file)
  */
void DataWidget::importBBFile(QFile & file) {
   //preparation of the progress bar
   QProgressDialog progress(tr("Parsing BB file..."), tr("Abort"), 0, 100, this);
   progress.setWindowModality(Qt::WindowModal);
   progress.show();
   progress.setValue(0);

   // clear the current data
   clearDataImmediate();

   // preparation in the data structure:
   Object * BBobject;
   Category * BBcat;
   QRect BBrect;
   BBox box;
   BBcat = addCategory (QString("Objects"));
   // variables for reading the file:
   QTextStream data (&file);
   int lineNo =0;
   int dataFrameCount = data.readLine().toInt();
   progress.setMaximum(dataFrameCount);
   videofileInfo.framecount = dataFrameCount;
   lineNo++;
   int frameNo;
   int objCount;
   int bboxCount;
   bool ok0, ok1, ok2, ok3;
   QStringList rectData;
   for (int i=0; i<dataFrameCount; i++){

      progress.setValue(i);
      if (progress.wasCanceled()) {
         clearDataImmediate();
         return;
      }

      // read and check frame number
      frameNo = int(data.readLine().toInt());
      //std::cout << "Processing frame number" << frameNo << std::endl;
      lineNo++;
      if (frameNo != i){
         QMessageBox::warning(this, tr("Error reading BB File"), tr("File\n\"")
                                                                 +filename
                                                                 +tr("\"\ncontained wrong frame number in Frame.")
                                                                 +QString("%1").arg(i)
                                                                 +tr(" in Line ")
                                                                 +QString("%1").arg(lineNo)
                                                                );
         return;
      }
      // read object number for this frame
      objCount = int(data.readLine().toInt());
      lineNo++;
      // loop over all objects and  for each
      for (int objNo =0; objNo < objCount; objNo++){
        BBobject = new Object();
        // read lifetime number in frames
        //std::cout <<"printing object number: " << objNo << std::endl;
        bboxCount = int(data.readLine().toInt());
        lineNo++;
        for (int currentBbox=0; currentBbox < bboxCount ; currentBbox++){
            // add qrect here
            rectData = (data.readLine()).split(";");
            lineNo++;
            // create bbox
            BBrect = QRect(int((rectData.value(0)).toDouble(&ok0)),
                           int((rectData.value(1)).toDouble(&ok1)),
                           int((rectData.value(2)).toDouble(&ok2)),
                           int((rectData.value(3)).toDouble(&ok3))
                         );
            // check if bbox was well constructed
            if (!(ok0 && ok1 && ok2 && ok3)){
               QMessageBox::warning(this, tr("Error Reading Data"), tr("Error reading Data in File \"")
                                                                     +filename
                                                                     +tr("\" in line ")
                                                                     +QString("%1").arg(lineNo)
                                                                     +tr(":\n Error parsing BBox data"));
                 return;
            }
            // add bbox with category
            //construct BBox:
            box = BBox(i+currentBbox, BBrect, BBobject->getID(), BBox::SINGLE);
            // add to list first, because bb file stores them with ascending filenumbers!, instead of adding directly
            BBobject->addBBox(box);

         }
         BBcat->addObject(BBobject);
      }
   }
   progress.setValue(dataFrameCount);
}

/** If the reading or parsing fails at any stage before tracking data could be
  * read the function simply aborts.
  * @note Since the ViPER format has far more potential than we need some
  * informations simply get omitted. This inflicts the whole config node, all
  * but one sourcefile nodes, file and content nodes, others than the first
  * attribute node of an object and finally all other data nodes but data::bbox
  * nodes.
  * @note The official definition of the ViPER file format can be viewed here:
  * <a href="http://viper-toolkit.sourceforge.net/docs/file/">ViPER XML</a>
  * @sa void importFile()
  * @sa void importBBFile(QFile & file)
  */
void DataWidget::importViperFile(QFile & file) {
   QProgressDialog progress(tr("Parsing ViPER file..."), tr("Abort"), 0, 100, this);
   progress.setWindowModality(Qt::WindowModal);
   progress.show();
   progress.setValue(0);

   QDomDocument doc(tr("ViPER file"));
   // load file in QDomDocument
   if (!doc.setContent(&file)) {
      file.close();
      QMessageBox::warning(this, tr("Invalid XML file"), tr("The file\n\"")
                                                         +file.fileName().section('/', -1)
                                                         +tr("\"\nis not a valid XML file!"));
      return;
   }

   QDomElement viperElem = doc.documentElement();
   if (viperElem.tagName() != QString("viper")) {
      QMessageBox::warning(this, tr("Not a ViPER file"), tr("The file\n\"")
                                                         +file.fileName().section('/', -1)
                                                         +tr("\"\nis not a ViPER file!"));
      return;
   }

   QDomElement dataElem = viperElem.firstChildElement(QString("data"));
   if (dataElem.isNull()) {
      QMessageBox::warning(this, tr("Not a valid ViPER file"), tr("The file\n\"")
                                                               +file.fileName().section('/', -1)
                                                               +tr("\"\nis not a valid ViPER file!"));
      return;
   }

   QDomElement sourcefileElem = dataElem.firstChildElement(QString("sourcefile"));
   if (sourcefileElem.isNull()) {
      QMessageBox::warning(this, tr("Not a valid ViPER file"), tr("The file\n\"")
                                                               +file.fileName().section('/', -1)
                                                               +tr("\"\nis not a valid ViPER file!"));
      return;
   }

   videofileInfo.filename = sourcefileElem.attribute(QString("filename"));

   int progressMax = sourcefileElem.childNodes().size();
   progress.setMaximum(progressMax);
   int counter = 1;

   // clear the current data
   clearDataImmediate();

   QDomElement objectElem = sourcefileElem.firstChildElement(QString("object"));
   Object * object;
   while (!objectElem.isNull()) {
      object = new Object(objectElem);

      if (object->isEmpty()) {
         delete object;
         object = NULL;
      }
      else {
         addObject(object, objectElem.attribute(QString("name")));
      }

      progress.setValue(counter++);
      if (progress.wasCanceled()) {
         clearDataImmediate();
         return;
      }

      objectElem = objectElem.nextSiblingElement(QString("object"));
   }
   progress.setValue(progressMax);

   // request the corresponding video file
   if (!videofileInfo.filename.isEmpty()) {
      emit requestVideo(QDir(videofileInfo.filename).absolutePath());
   }
}

/** This function is used to keep selection dependent actions in sync
  */
bool DataWidget::isObjectSelected() const {
   Cell cell = getSelection();
   return (cell.index>=0 && cell.row>=0);
}

QSize DataWidget::minimumSizeHint() const {
   return QSize(256, 192);
}

/** The categorie's name gets asked from the user via a QInputDialog. If a
  * category with the same name allready exists or the user aborts the dialog
  * nothing is done.
  * @sa void deleteCategory()
  */
void DataWidget::newCategory() {
   bool ok;
   QString name = QInputDialog::getText(this, tr("Name"),
                                        tr("Name of the new category:"),
                                        QLineEdit::Normal,
                                        QString(), &ok);
   if (ok && !name.isEmpty()) {
      bool exists = false;
      int i=0;
      while(i<categories.size() && !exists) {
         exists = (categories.at(i++)->getName() == name);
      }
      if (exists) {
         --i;
      }
      else {
         Category * cat = addCategory(name);
         cat->addObject(new Object());
      }
      // show the category
      setSelection(i, 0, currentFrameNr);
   }
}

/** Internally simply the corresponding function of the shown categorie is
  * called.
  * @sa void Category::newObject()
  */
void DataWidget::newObject() {
   if (count() > 1) {
      categories.at(currentIndex())->newObject();
   }
   else {
      addObject(new Object, tr("Default"));
   }
   setSelection(currentIndex(), categories.at(currentIndex())->getObjects().size()-1, currentFrameNr);
}

/** This is used to catch the case that the last tab gets focus. In this case
  * the focus is set to the first index and a new category is created. ("new tab"
  * tab behavior)
  * @sa void newCategory()
  */
void DataWidget::onCurrentTabChanged(int index) {
   if (count()>1 ) {
      if (index == count()-1) {
         setCurrentIndex(0);
         newCategory();
      }
      else {
         static_cast<QTableView *>(currentWidget())->selectionModel()->clear();
         setSelection(currentIndex(), 0, currentFrameNr);
      }
   }
}

/** If anything bad happens before any data can be read the function aborts with
  * a warning, else the old data is cleared an the new is read from th file.
  * @note The filename is saved in \ref filename for saveFile()
  */
void DataWidget::openFile() {
   // Get a filename from the user
   QString openFilename = QFileDialog::getOpenFileName(this,
                                                       tr("Open data file"),
                                                       QString(),
                                                       tr("Binary tracking data (*.btd)"));
   if (openFilename.isEmpty()) {
      return;
   }

   QDir::setCurrent(openFilename.section('/', 0, -2));

   //Open the file
   QFile file(openFilename);
   if (!file.open(QIODevice::ReadOnly)) {
      QMessageBox::warning(this, tr("Unable to open file"), tr("The file\n\"")
                                                            +openFilename.section('/', -1)
                                                            +tr("\"\ncouldn't be opened!"));
      return;
   }
   QDataStream in(&file);
   quint8 tempByte;
   in >> tempByte;
   if (tempByte != (quint8)'B') {
      QMessageBox::warning(this, tr("Invalid file"), tr("The file\n\"")
                                                     +openFilename.section('/', -1)
                                                     +tr("\"\nis not a valid BTD file!"));
      return;
   }
   in >> tempByte;
   if (tempByte != (quint8)'T') {
      QMessageBox::warning(this, tr("Invalid file"), tr("The file\n\"")
                                                     +openFilename.section('/', -1)
                                                     +tr("\"\nis not a valid BTD file!"));
      return;
   }
   in >> tempByte;
   if (tempByte != (quint8)'D') {
      QMessageBox::warning(this, tr("Invalid file"), tr("The file\n\"")
                                                     +openFilename.section('/', -1)
                                                     +tr("\"\nis not a valid BTD file!"));
      return;
   }
   in >> tempByte;
   if (tempByte != (quint8)1) {
      QMessageBox::warning(this, tr("Wrong version"), tr("The file\n\"")
                                                      +openFilename.section('/', -1)
                                                      +tr("\"\nhas a not supported version!"));
      return;
   }

   // point of no return
   clearDataImmediate();

   filename = openFilename;

   in >> videofileInfo.filename;

   quint32 id;
   in >> id;
   idCounter->reset(id);

   quint32 size;
   in >> size;
   for (quint32 i=0; i<size; ++i) {
      addCategory(new Category(in));
   }

   // request the corresponding video file
   if (!videofileInfo.filename.isEmpty()) {
      emit requestVideo(QDir(videofileInfo.filename).absolutePath());
   }

   updateFramecount();
}

/** The filename is taken from \ref filename. If this is empty saveFileAs() is called
  * @sa void saveFileAs()
  */
void DataWidget::saveFile() {
   if (filename.isEmpty()) {
      saveFileAs();
      return;
   }

   // create/open file
   QFile file(filename);
   if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      QMessageBox::warning(this, tr("Unable to access file"), tr("Unable to access file\n\"")
                                                              +filename.section('/', -1)
                                                              +tr("\"\nfor writing!"));
      return;
   }
   QDataStream out(&file);

   out << (quint8)'B';
   out << (quint8)'T';
   out << (quint8)'D';
   out << (quint8)1;
   out << videofileInfo.filename;
   out << (quint32)idCounter->getID();
   out << (quint32)categories.size();
   foreach (Category const * const category, categories) {
      category->save(out);
   }
}

/** The filename is asked from the user and saved in \ref filename
  */
void DataWidget::saveFileAs() {
   QString btdFilter = tr("Binary tracking data (*.btd)");
   // get filename from user
   QString saveFilename = QFileDialog::getSaveFileName(this,
                                                       tr("Save data"),
                                                       filename,
                                                       btdFilter,
                                                       &btdFilter);
   if (saveFilename.isEmpty()) {
      return;
   }
   // check for suffix
   if(!saveFilename.endsWith(".btd")) {
      saveFilename.append(".btd");
   }
   filename = saveFilename;
   QDir::setCurrent(filename.section('/', 0, -2));
   saveFile();
}

/** The QModelIndex from the signal gets converted to the corresponding objects
  * unique ID. The signal selectionChanged(int id) then gets emitted with said
  * ID.
  */
void DataWidget::selectionChanged(QModelIndex const & current, QModelIndex const &) {
   int newFrameNr = currentFrameNr;
   int newObjectID = -1;
   if (current.isValid()) {
      newFrameNr = current.column();
      newObjectID = current.data(Qt::UserRole).toInt();
   }

   if (currentFrameNr != newFrameNr) {
      currentFrameNr = newFrameNr;
      emit currentFrameChanged(currentFrameNr);
      emit updateActions();
   }

   if (selectedObjectID != newObjectID) {
      selectedObjectID = newObjectID;
      emit selectedObjectChanged(selectedObjectID);
      emit updateActions();
   }
}

void DataWidget::selectNextCategory() {
   if (count() > 1 && currentIndex() < count()-2) {
      setSelection(currentIndex()+1, 0, currentFrameNr);
   }
   else if (currentIndex()==count()-2) {
      setCurrentIndex(count()-1);
   }
}

/**
  * @note Keyframe refers to a frame containing a key box for the current object
  */
void DataWidget::selectNextKeyframe() {
   Cell cell = getSelection();
   if (!cell.isNull()) {
      int i = cell.column;
      bool found = false;
      Object * object = categories.at(cell.index)->getObjects().at(cell.row);
      BBox * bbox;
      while (i<videofileInfo.framecount && !found) {
         bbox = object->getBBoxPointer(++i);
         found = (bbox && bbox->type==BBox::KEYBOX);
      }
      if (found) {
         setSelection(cell.index, cell.row, i);
      }
   }
}

void DataWidget::selectNextObject() {
   Cell cell = getSelection();
   if (!cell.isNull() && cell.row+1<categories.at(currentIndex())->rowCount()) {
      setSelection(cell.index, cell.row+1, cell.column);
   }
}

void DataWidget::selectPreviousCategory() {
   if (count() > 1 && currentIndex() > 0) {
      setSelection(currentIndex()-1, 0, currentFrameNr);
   }
}

/**
  * @note Keyframe refers to a frame containing a key box for the current object
  */
void DataWidget::selectPreviousKeyframe() {
   Cell cell = getSelection();
   if (!cell.isNull()) {
      int i = cell.column;
      bool found = false;
      Object * object = categories.at(cell.index)->getObject(selectedObjectID);
      while (i>0 && !found) {
         found = object->getBBox(--i).type==BBox::KEYBOX;

      }
      if (found) {
         setSelection(cell.index, cell.row, i);
      }
   }
}

void DataWidget::selectPreviousObject() {
   Cell cell = getSelection();
   if (!cell.isNull() && cell.row>0) {
      setSelection(cell.index, cell.row-1, cell.column);
   }
}

/** @sa void currentFrameChanged(int framenumber)
  * @sa void VideoWidget::seek(int frame)
  * @sa void VideoWidget::currentFrameChanged(int n);
  */
void DataWidget::setCurrentFrame(int framenumber) {
   if (count()>1 && currentFrameNr != framenumber) {
      Cell cell = getSelection();
      if (!cell.isNull()) {
         setSelection(cell.index, cell.row, framenumber);
      }
      else {
         setSelection(currentIndex(), -1, framenumber);
      }
   }
}

/** @sa void selectionChanged(int id);
  * @sa void GLWidget::changeSelection(int id)
  * @sa void GLWidget::selectionChanged(int id)
  */
void DataWidget::setSelectedObject(int id) {
   if (count()>1 && selectedObjectID != id) {
      int catNr = currentIndex();
      int row = -1;

      if (id >= 0) {
         bool found = false;
         int i = 0;
         while (!found && i<categories.size()) {
            int j = categories.at(i)->findObject(id);
            if (j >= 0) {
               catNr = i;
               row = j;
               found = true;
            }
            ++i;
         }
      }

      setSelection(catNr, row, currentFrameNr);
   }
}

/** @note This doesn't use setSelection and therefore doesn't scroll to the new
  *       selection
  */
void DataWidget::setSelectedObjectByRow(int row) {
   QTableView * tableView = static_cast<QTableView *>(currentWidget());
   QModelIndex index = categories.at(currentIndex())->index(row, currentFrameNr);
   if (index.isValid()) {
      tableView->setCurrentIndex(index);
   }
}

/** This is used internally to change the selection and scrolls the view to center
  * the new selection.
  */
void DataWidget::setSelection(int tab, int row, int column) {
   QTableView * tableView = static_cast<QTableView *>(widget(tab));
   QModelIndex index = categories.at(tab)->index(row, column);
   setCurrentIndex(tab);
   if (index.isValid()) {
      tableView->setCurrentIndex(index);
      tableView->scrollTo(index, QAbstractItemView::PositionAtCenter);
   }
   else {
      currentFrameNr = column;
      tableView->selectionModel()->clear();
      tableView->scrollTo(categories.at(currentIndex())->index(0, currentFrameNr), QAbstractItemView::PositionAtCenter);
   }
}

/** This is used so the VideoWidget can tell the DataWidget about a newly loaded
  * videos properties.
  */
void DataWidget::setVFInfo(VideofileInfo const & newVideofileInfo) {
   videofileInfo = newVideofileInfo;
   foreach (Category * const category, categories) {
      category->setColumnCount(videofileInfo.framecount);
   }
   if (count()>1) {
      setSelection(0, 0, 0);
   }
}

/** Actually every category is told to sort itself
  * @sa void Category::sortByFN()
  */
void DataWidget::sortByFN() {
   foreach (Category * category, categories) {
      category->sortByFN();
   }
}

/** Actually every category is told to sort itself
  * @sa void Category::sortByID()
  */
void DataWidget::sortByID() {
   foreach (Category * category, categories) {
      category->sortByID();
   }
}

/** This is used to get a default width for the views if no video file is opened.
  */
void DataWidget::updateFramecount() {
   if (!videofileInfo.size.isValid()) {
      videofileInfo.framecount = 0;
      foreach (Category * const category, categories) {
         videofileInfo.framecount = qMax(videofileInfo.framecount,
                                         category->getFramecount());
      }
      foreach (Category * const category, categories) {
         category->setColumnCount(videofileInfo.framecount);
      }
   }
}

/** The Pixmap is in grayscale and 50% transparent.
  */
QPixmap inactivePixmap(QString const & name) {
   QImage image(name);
   QRgb * pixel = (QRgb *)image.bits();
   int gray;
   for (int i=0; i<image.width()*image.height(); ++i) {
      gray = qGray(*pixel);
      (*pixel) = qRgba(gray, gray, gray, qAlpha(*pixel)/2);
      ++pixel;
   }
   return QPixmap::fromImage(image);
}
