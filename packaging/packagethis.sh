#!/bin/bash
#execute this file from ../trackit, meaning the parent directory of TrackIt
inputdir=$1
packagedir=$2

echo "Trackit .tar.gz-package generator"

if [ -z $inputdir ]
then
   echo "No input directory specified";
   exit;
fi

if [ -z $packagedir ] 
then
   echo "No package dir specified";
   exit;
fi

if [ ! -d $inputdir ] 
then
  echo "Inputdirectory does not exist - aborting"
  exit;
fi

if [[ -d $packagedir || -f $packagedir ]] 
then
  echo "Input directory already exists - aborting"
  exit;
fi

echo "Inputdirectory is: $inputdir"
echo "Outputdirectory is: $packagedir"


echo "Generating directory structure"
mkdir $packagedir
mkdir $packagedir/doc

echo "Copying documentation (did you make sure you created doxygen and pdflatex docs?)"
# copy doc files:
# execute doxygen doku before (todo)
cp -r $inputdir/doc/html $packagedir/html
cp $inputdir/doc/manual.pdf $packagedir/doc
cp $inputdir/ReadMe.txt $packagedir/ReadMe.txt

# generate html forwarding
# '<html><head><meta http-equiv="refresh" content="0; url=html/index.html"></head><body><a href="html/index.html">Redirect to documentation</a></body></html>' > $packagedir/doc/DoxyDoku.html
# escape html: ed 's/&/\&amp;/g; s/</\&lt;/g; s/>/\&gt;/g; s/"/\&quot;/g; s/'"'"'/\&#39;/g'
echo '&lt;html&gt;&lt;head&gt;&lt;meta http-equiv=&quot;refresh&quot; content=&quot;0; url=html/index.html&quot;&gt;&lt;/head&gt;&lt;body&gt;&lt;a href=&quot;html/index.html&quot;&gt;Redirect to documentation&lt;/a&gt;&lt;/body&gt;&lt;/html&gt;' > $packagedir/doc/DoxyDoku.html

echo "copying ressources"
#copy ressources
cp -r $inputdir/svg $packagedir
cp -r $inputdir/icons $packagedir
cp $inputdir/icons.qrc $packagedir
cp $inputdir/TrackIt.pro $packagedir

echo "copying source files"
# copy sources
cp $inputdir/*.h $packagedir
cp $inputdir/*.cpp $packagedir
rm $packagedir/*.moc.cpp

echo "generating tar file"
if [ -e TrackIt.tar.gz ]
then
   echo "Aborting - TrackIt.tar.gz already exists"
   exit
fi
tar czfv TrackIt.tar.gz $packagedir

### Follow these instructions to create a debian package
# bzr dh-make trackit 1.0 TrackIt-1.0.tar.gz
# cd trackit
# rm *.ex
# rm *.EX
# cd debian
# vi changelog
## add:
## trackit (1.0-0ubuntu1) unstable; urgency=low
##
##  * Initial release 
##
## -- Hp Haegele  <haegelhr@studi.informatik.uni-stuttgart.de>  Thu, 02 May 2013 12:58:32 +0200

## vi control
## insert:
# Source: trackit
# Section: video
# Priority: extra
# Maintainer: Hanspeter Haegele <haegelhr@studi.informatik.uni-stuttgart.de>
# Build-Depends: debhelper (>= 7.0.50~), libopencv-core (>= 2.3), libhighgui (>= 2.3)
# Standards-Version: 3.8.4
# Homepage: 
# #Vcs-Git: git://git.debian.org/collab-maint/trackit.git
# #Vcs-Browser: http://git.debian.org/?p=collab-maint/trackit.git;a=summary
# 
# Package: trackit
# Architecture: any
# Depends: libopencv-core (>= 2.3), libhighgui (>= 2.3), libqt4-xml (>= 4.7.0), libqt4-opengl (>= 4.7.0), libqt4-core (>= 4.7.0), libqt4-gui (>= 4.7.0)
# Description:  TrackIt is an easy manual bounding box editor vor videos,
#  also supports ViPER xml files.
#
# here: Readme files etc.. 

# dann:
#  cd ..
#  bzr builddeb -- -us -uc
# und weiter mit: http://developer.ubuntu.com/packaging/html/packaging-new-software.html
