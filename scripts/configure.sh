
function configure()
{
	local PACKAGE_NAME=$1
	local DISTRIB=$2

	set +e
	mkdir -p $OUT_DIR/build/$OS_V/uhuru-av/$PACKAGE_NAME

	set -e
	cd $SRC_DIR/$PACKAGE_NAME
	sudo chmod +x autogen.sh
	./autogen.sh	

	cd $OUT_DIR/build/$OS_V/uhuru-av/$PACKAGE_NAME
	echo "-------CONFIGURE-------"
	$SRC_DIR/$PACKAGE_NAME/configure --prefix=$OUT_DIR/install/$OS_V/uhuru-av PKG_CONFIG_PATH=$OUT_DIR/install/$OS_V/uhuru-av/lib/pkgconfig
	echo "-------CONFIGURE OKAY $PACKAGE_NAME-------"

}

## Build pour Unbutu-14.04-64
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
OS_V=ubuntu-14.04-64

OUT_DIR=$DIR/../out
SRC_DIR=$DIR/../

function usage()
{
    echo "Usage: configure.sh -p PACKAGE -d DISTRIB"
    echo "Configure package given before compiling"
    echo "Example: configure.sh -p modules/clamav -d trusty"
    echo ""
    echo "Argument:"
    echo "  PACKAGE          Package subpath to configure"
    echo "  DISTRIB          Distribution of the package"
    echo ""

    exit 1
}

while getopts "p:d:h" opt; do
    case $opt in
	p)
	    PACKAGE=$OPTARG
	;;
	d)
	    DISTRIB=$OPTARG
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

configure $PACKAGE $DISTRIB

# mkdir -p $OUT_DIR/build/$OS_V/uhuru-av/libuhuru/pkgconfig ??

## On lui indique où trouver les bases
#set +e
#mkdir -p $OUT_DIR/install/$OS_V/uhuru-av/var/lib/uhuru/
#ln -s /var/lib/uhuru/bases/ $OUT_DIR/install/$OS_V/uhuru-av/var/lib/uhuru/bases


