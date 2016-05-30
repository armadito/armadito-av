#!/bin/bash

function usage()
{
    echo "Usage: deb-src.sh -d DISTRIB -k GPG_KEY_ID [-b BUILD_VERSION] TARBALL"
    echo "Create a Debian source package from tarball"
    echo "Example: deb-src.sh -d trusty -k BEEFFACE armadito-core-0.10.1.tar.gz"
    echo ""
    echo "Argument:"
    echo "  TARBALL          source tarball"
    echo ""
    echo "Options:"
    echo "  DISTRIB          distrib code name (trusty, wily, xenial...)"
    echo "  GPG_KEY_ID       GPG key id for package signing"
    echo "  BUILD_VERSION    build version appended to source version"
    echo ""

    exit 1
}

function mkdir_if_needed()
{
    local dir
    
    for dir in $* ; do
	[ -d $dir ] || mkdir -p $dir
    done
}

# extract the package name from a tarball name, as in
# $ pkg libfoo-1.2.3.tar.gz
# libfoo
function pkg()
{
    local P=$(basename $1 | sed -r -e 's/-[0-9][0-9]*.[0-9][0-9]*.[0-9][0-9]*.tar.((gz)|(bz2))$//')
    echo $P
}

# extract the version from a tarball name, as in
# $ pkg libfoo-1.2.3.tar.gz
# 1.2.3
function version()
{
    local V=$(basename $1 | sed -r -e 's/^[^\.]*([0-9][0-9]*.[0-9][0-9]*.[0-9][0-9]*).tar.((gz)|(bz2))$/\1/g')
    echo $V
}

# extract the extension from a tarball name, as in
# $ pkg libfoo-1.2.3.tar.gz
# gz
function tarball_extension()
{
    local E=$(basename $1 | sed -r -e 's/^[^\.]*[0-9][0-9]*.[0-9][0-9]*.[0-9][0-9]*.tar.((gz)|(bz2))$/\1/g')
    echo $E
}

function build_deb_src()
{
    local TARBALL=$1
    local DISTRIB=$2
    local GPG_KEY=$3
    local BUILD_VERSION=$4

    local OUT_DIR=$SCRIPT_DIR/../out
    local PKG=$(pkg $TARBALL)
    local TARBALL_EXT=$(tarball_extension $TARBALL)
    local SRC_VERSION=$(version $TARBALL)
    if [ ! -z "$BUILD_VERSION" ] ; then
	VERSION=$SRC_VERSION.$BUILD_VERSION
    else
	VERSION=$SRC_VERSION
    fi

    local DEBIAN_TARBALL=${PKG}_$VERSION.orig.tar.$TARBALL_EXT
    local BUILD_DIR=$OUT_DIR/pkg
    local DEBIAN_DIR=$SCRIPT_DIR/../packages/ubuntu/$PKG/debian

    mkdir_if_needed $BUILD_DIR

    /bin/rm -rf $BUILD_DIR/$PKG-$VERSION

    if test "$TARBALL_EXT" = 'gz' ; then TAR_FLAG=z ; else TAR_FLAG=j ; fi

    cp $TARBALL $BUILD_DIR/$DEBIAN_TARBALL
    
    (
	cd $BUILD_DIR
	tar xv${TAR_FLAG}f $DEBIAN_TARBALL

	if [ ! -z "$BUILD_VERSION" ] ; then
	    mv $BUILD_DIR/$PKG-$SRC_VERSION $BUILD_DIR/$PKG-$VERSION
	fi
    )

    cp -r $DEBIAN_DIR $BUILD_DIR/$PKG-$VERSION

    # we append the distro to the package version
    (
	cd $BUILD_DIR/$PKG-$VERSION

	if [ ! -z "$BUILD_VERSION" ] ; then
	    local DEBIAN_VERSION=$(dpkg-parsechangelog --show-field Version | sed -r -e "s/$SRC_VERSION//") 
	    dch --newversion $VERSION$DEBIAN_VERSION --distribution $DISTRIB --controlmaint "$BUILD_VERSION"
	fi

	dch --local ppa1~$DISTRIB --distribution $DISTRIB --controlmaint 'adding distro name'

	debuild -S -sa -pgpg2 -k$GPG_KEY_ID
    )
}

#function upload_one()
#{
#}

if [ $# -eq 0 ] ; then usage ; fi

while getopts "d:k:b:h" opt; do
    case $opt in
	d)
	    DISTRIB=$OPTARG
	    ;;
	k)
	    GPG_KEY_ID=$OPTARG
	    ;;
	b)
	    BUILD_VERSION=$OPTARG
	    ;;
	h)
	    usage
	    ;;
	\?)
	    usage
	    ;;
	:)
	    usage
	    ;;
    esac
done
shift $((OPTIND-1))

set -x

TARBALL=$1

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

build_deb_src $TARBALL $DISTRIB $GPG_KEY_ID $BUILD_VERSION

exit 0

# ===================
dput -u ppa:armadito/armadito-av $PKGDIR/${PKG}_$VERSION-${BUILDVER}_source.changes




