#ifndef JULIA_H
#define JULIA_H

#include <complex>
#include <QtGui/QLabel>

class QLabel;

class Julia : public QLabel {

   Q_OBJECT

public:
   explicit Julia(QWidget * parent = 0);

public slots:
   void exec();

protected:
   virtual void mousePressEvent(QMouseEvent * event);

private:
   void calc(std::complex<double> c);
};

void juliaSubImage(QImage * image, int yStart, int yEnd, std::complex<double> c);
void juliaPixel(QRgb * pixel, std::complex<double> z);
int escapeTime(std::complex<double> z, std::complex<double> const & c, int nMax,  double zMax);

#endif // JULIA_H
