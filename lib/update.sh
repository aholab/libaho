#!/bin/sh

mkdir include
mkdir doc


cp /home/dos/prj/libaho/lib/*.a .
cp /home/dos/prj/libaho/libdos/{debug,release}/*.lib .
cp /home/dos/prj/libaho/libwin/{debug,release}/*.lib .
cp /home/dos/prj/libaho/doc/* doc
cp /home/dos/prj/libaho/libdos/include/* include

find . -type "d" -exec chmod 755 {} \;
find . -type "f" -exec chmod 644 {} \;

