#!/bin/bash

#echo $$_PRO_FILE_PWD_ $$OUT_PWD; mkdir uhuru-$$UHURU_VERSION; for f in $$SOURCES $$HEADERS; do (cd $$_PRO_FILE_PWD_; cp -r $$f $$OUT_PWD/uhuru-$$UHURU_VERSION; ) ; done;
# tar cvzf uhuru-$${UHURU_VERSION}.tar.gz uhuru-$$UHURU_VERSION; /bin/rm -rf uhuru-$$UHURU_VERSION

set -x

DISTDIR=$1
SRCDIR=$2
DESTDIR=$3

shift 3

[ -d $DISTDIR ] && /bin/rm -rf $DISTDIR
mkdir $DISTDIR
for f in $* ; do
    echo $f
    df=$(dirname $f)
    bf=$(basename $f)
    if test $df == '.'; then
        ( cd $SRCDIR; cp $f $DESTDIR/$DISTDIR/$bf )
    else
	mkdir -p $DISTDIR/$df
	( cd $SRCDIR; cp $f $DESTDIR/$DISTDIR/$df/$bf )
    fi
done
tar cvzf $DISTDIR.tar.gz $DISTDIR
