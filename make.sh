#!/bin/sh

cp mt7621.config .config

DATESTR=`date +%Y%m%d%H%M`

for i in build_config/*.conf; do
. $i
echo $board

DIST=$board
OSTYPE=$os_type
sed -i "s/option hostname.*$/option hostname $DIST/" ./package/base-files/files/etc/config/system
sed -i "s/CONFIG_VERSION_DIST=\".*\"/CONFIG_VERSION_DIST=\"$DIST\"/" ./.config
sed -i "s/CONFIG_VERSION_NUMBER=\".*\"/CONFIG_VERSION_NUMBER=\"v2.1-$DATESTR\"/" ./.config
touch ./package/base-files/files/etc/openwrt_release

make

test $? -eq 0 || exit 1

DIST=`cat .config | grep CONFIG_VERSION_DIST | awk -F\" '{print $2}'`
VERSION=`cat .config | grep CONFIG_VERSION_NUMBER | awk -F\" '{print $2}'`

mkdir -p bin/mt7621/$OSTYPE/$DATESTR
echo mv bin/ramips-glibc/*$VERSION*puppies*bin bin/mt7621/$OSTYPE/$DATESTR/$DIST-$VERSION
mv bin/ramips-glibc/*$VERSION*puppies*bin bin/mt7621/$OSTYPE/$DATESTR/$DIST-$VERSION

done
