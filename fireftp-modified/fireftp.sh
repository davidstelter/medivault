#!/bin/bash

builddir=$HOME/Projects/fireftp-modified/
chrome=$builddir/chrome/


cd $chrome
zip -r fireftp content locale skin
mv fireftp.zip ./fireftp.jar 

cd $builddir
zip -r fireftp chrome components defaults platform chrome.manifest install.rdf license.txt
mv fireftp.zip ./fireftp.xpi
