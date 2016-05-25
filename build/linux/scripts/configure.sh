
function configure()
{
	local PACKAGE_NAME=$1
	local OS_V=$2

	set +e
	mkdir -p $OUT_DIR/build/$OS_V/armadito-av/$PACKAGE_NAME

	set -e
	cd $SRC_DIR/$PACKAGE_NAME
	sudo chmod +x autogen.sh
	./autogen.sh	

	cd $OUT_DIR/build/$OS_V/armadito-av/$PACKAGE_NAME
	echo "-------CONFIGURE-------"
	$SRC_DIR/$PACKAGE_NAME/configure --prefix=$OUT_DIR/install/$OS_V/armadito-av PKG_CONFIG_PATH=$OUT_DIR/install/$OS_V/armadito-av/lib/pkgconfig
	echo "-------CONFIGURE OKAY $PACKAGE_NAME-------"

}

## Build pour Unbutu-14.04-64
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

OUT_DIR=$DIR/../out
SRC_DIR=$DIR/../../../

function usage()
{
    echo "Usage: configure.sh -p PACKAGE -o OS_V"
    echo "Configure package given before compiling for a specific OS version"
    echo "Example: configure.sh -p modules/clamav -o ubuntu-14.04-64"
    echo ""
    echo "Argument:"
    echo "  PACKAGE          Package subpath to configure"
    echo "  OS_V             OS version out subdirectory name"
    echo ""

    exit 1
}

while getopts "p:o:h" opt; do
    case $opt in
	p)
	    PACKAGE=$OPTARG
	;;
	o)
	    OS_V=$OPTARG
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

configure $PACKAGE $OS_V

