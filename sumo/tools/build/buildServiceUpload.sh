#!/bin/bash
PREFIX=$1
NIGHTDIR=/misc/nas/Daten/Sumo/Nightly
for f in $PREFIX/sumo/sumo*.tar.bz2; do
  if test -e $PREFIX/sumo/configure -a $f -nt $PREFIX/sumo/configure; then
    curl -n -X PUT -T $PREFIX/sumo/build/package/sumo.spec https://api.opensuse.org/source/home:behrisch/sumo_nightly/sumo.spec
    curl -n -X PUT -T $PREFIX/sumo/`basename $f` https://api.opensuse.org/source/home:behrisch/sumo_nightly/`basename $f`
    BASE=`basename $f .tar.bz2`
    NIGHTFILE=$NIGHTDIR/sumo-msvc8Win32-bin.zip
    if test -e $NIGHTFILE; then
      cd /tmp
      rm -rf $BASE
      tar xjf $f $BASE/examples $BASE/tools
      mkdir $BASE/bin
      cd $BASE/bin
      unzip -q $NIGHTFILE
      cd ../..
      zip -rqmT $NIGHTDIR/sumo-winbin${BASE:4}.zip $BASE
    fi
  fi
done
for f in $PREFIX/sumo/sumo-*.tar.* $PREFIX/sumo/sumo-*.zip; do
  if test -e $PREFIX/sumo/configure -a $f -nt $PREFIX/sumo/configure; then
    cp $f $NIGHTDIR
  fi
done
