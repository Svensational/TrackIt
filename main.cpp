#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include "mainwindow.h"

int main(int argc, char * argv[])
{
   QApplication a(argc, argv);
   MainWindow w;
   QRect desktop = a.desktop()->availableGeometry();
   if (desktop.width()<=800 || desktop.height()<=600) {
      // If the window would be bigger than the screen show it maximized
      w.showMaximized();
   }
   else {
      // else show it centered on the screen
      w.show();
      w.setGeometry((desktop.width()-800)/2, (desktop.height()-600)/2, 800, 600);
   }

   return a.exec();
}
