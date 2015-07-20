This is the TrackIt Readme file
TrackIt v1.0 - 25.04.2013 
Softwarepraktikum 2012/2013 

at
University of Stuttgart 
Visualisation Research Centre
www.vis.uni-stuttgart.de
kuno.kurzhals@vis.uni-stuttgart.de

by
Sven Klingel (sven.klingel@googlemail.com)
Hanspeter Haegele (haegelhr@studi.informatik.uni-stuttgart.de)


Overview:
+-------------------------+
  -TrackIt
  -System Requirements
  -Installation
  -Documentation
  -Developer Documentation

TrackIt:
+-------------------------+
TrackIt is a program to help with easy creation bounding boxes on video files to create ground truth data for those files. 

System Requirements:
+-------------------------+
Windows: 
At least Windows Xp, 256mb ram

Linux:
Packages available for Debian (64 bit)
For compiling:
-Gcc build environment
-OpenCV 2.4.2 or greater (development core and highgui modules)
-Qt 4.4 or greater (development packages)

Installation:
+-------------------------+
Windows:
Unzip into your working directory and start TrackIt.exe.

Linux:
Make sure your system meets the requirements.
Unzip into directory and open a terminal and type:
cd /path/to/your/directory
qmake-qt4
make
make install 

TrackIt is now installed and ready to use.

More information and documentation:
+-------------------------+
More documentation can be found in the trackit-directory/doc/manual.pdf in your TrackIt Directory.

Developer information:
+-------------------------+
Doxygen Documentation can be found in trackit-directory/doc/DoxyDoku.html


