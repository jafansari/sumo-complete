#!/bin/bash
PREFIX=$1
REMOTEDIR="behrisch,sumo@web.sourceforge.net:/home/groups/s/su/sumo/htdocs/daily/"
DOCDIR=$PREFIX/sumo_doxygen

cd $PREFIX/sumo
rm -rf $DOCDIR
nice -19 doxygen sumo.doxyconf
scp -qr $DOCDIR $REMOTEDIR
