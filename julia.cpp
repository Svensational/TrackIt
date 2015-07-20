#include "julia.h"
#include <QtGui/QBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QMouseEvent>
#include <QtConcurrentRun>

Julia::Julia(QWidget * parent) :
   QLabel(parent)
{
   setWindowTitle(QString("Julia f(z) = z^2 -0.8+0.2i"));
   QImage image(512, 512, QImage::Format_RGB32);
   setPixmap(QPixmap::fromImage(image));
}

void Julia::calc(std::complex<double> c) {
   const int xMax = 1024;
   const int yMax = 1024;

   QImage image(xMax, yMax, QImage::Format_RGB32);

   const int nThreads = QThreadPool::globalInstance()->maxThreadCount();
   for (int i=0; i<nThreads; ++i) {
      QtConcurrent::run(juliaSubImage, &image, (yMax/nThreads)*i, (yMax/nThreads)*(i+1), c);
   }
   /*for(int y=0; y<yMax; ++y) {
      QRgb * scanLine = (QRgb *)image.scanLine(y);
      for (int x=0; x<xMax; ++x) {
         int z = escapeTime(std::complex<double>((x-xMax/2.0)*4.0/double(xMax), (y-yMax/2.0)*4.0/double(yMax)), c, 128, 100.0)*2;
         scanLine[x] = qRgb(z, z, z);
      }
   }*/
   QThreadPool::globalInstance()->waitForDone();
   // poor MSAA
   setPixmap(QPixmap::fromImage(image.scaled(xMax/2, yMax/2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
   //setPixmap(QPixmap::fromImage(image));
}

void Julia::exec() {
   calc(std::complex<double>(-0.8, 0.2));
   show();
}

void Julia::mousePressEvent(QMouseEvent * event) {
   const std::complex<double> c(2*event->pos().x()/double(width())-1.0,
                                2*event->pos().y()/double(height())-1.0);
   setWindowTitle(QString("Julia f(z) = z^2 %1%2%3%4i").arg(c.real()>0?"+":"").arg(c.real(), 0, 'g', 2).arg(c.imag()>0?"+":"").arg(c.imag(), 0, 'g', 2));
   calc(c);
}

void juliaSubImage(QImage * image, int yStart, int yEnd, std::complex<double> c) {
   const int xMax = image->width();
   const int yMax = image->height();

   for(int y=yStart; y<yEnd; ++y) {
      QRgb * scanLine = (QRgb *)image->scanLine(y);
      for (int x=0; x<xMax; ++x) {
         int val = escapeTime(std::complex<double>((x-xMax/2.0)*4.0/double(xMax), (y-yMax/2.0)*4.0/double(yMax)),
                              c, 127, 2.0)*2;
         scanLine[x] = qRgb(val, val, val);
      }
   }
}

int escapeTime(std::complex<double> z, std::complex<double> const & c, int nMax, double zMax) {
   int n = 0;
   while (n<nMax && std::abs(z)<zMax) {
      z = z*z+c;
      ++n;
   }
   return n;
}
