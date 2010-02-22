#!/bin/bash

if [ ! -f 7za.exe ]; then
	echo "7za.exe not found..."
	exit -1
fi

zip="7za a -r -tzip"

cd chrome
../$zip fireftp.jar content locale skin

cd ..
./$zip fireftp.xpi chrome components defaults platform chrome.manifest install.rdf license.txt
