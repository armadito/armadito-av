
function configure()
{
	local PACKAGE_NAME=$1
	local OS_V=$2
	local SRC_DIR=$3
	local A_OPTS=""

	set -e
	mkdir -p $OUT_DIR/build/$OS_V/armadito-av/$PACKAGE_NAME

	set -e
	cd $SRC_DIR/
	sudo chmod +x autogen.sh
	./autogen.sh	

	if [[ $PACKAGE_NAME == "prelude" ]];
	then
		A_OPTS='--with-libprelude-pythondir=$HOME/prelude/install/lib/python2.7/site-packages --with-libprelude-libdir=$HOME/prelude/install/lib'
	fi

	if [[ $PACKAGE_NAME == "core" ]];
	then
		A_OPTS='--enable-debug --enable-fanotify'
	fi

	cd $OUT_DIR/build/$OS_V/armadito-av/$PACKAGE_NAME
	echo "-------CONFIGURE-------"
	$SRC_DIR/configure --prefix=$OUT_DIR/install/$OS_V/armadito-av PKG_CONFIG_PATH=$OUT_DIR/install/$OS_V/armadito-av/lib/pkgconfig $A_OPTS
	echo "-------CONFIGURE OKAY $PACKAGE_NAME-------"

}

## Build pour Ubuntu-14.04-64
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
OUT_DIR=$DIR/../out

function usage()
{
    echo "Usage: configure.sh -i SRC_DIR -p PACKAGE_NAME -o OS_V"
    echo "Configure package given before compiling for a specific OS version"
    echo "Example: configure.sh -i /home/joebar/armadito-mod-clamav -p clamav -o ubuntu-14.04-64"
    echo ""
    echo "Argument:"
    echo "  SRC_DIR              Sources directory"
    echo "  PACKAGE_NAME         Package name to configure"
    echo "  OS_V                 OS version out subdirectory name"
    echo ""

    exit 1
}

while getopts "p:i:o:h" opt; do
    case $opt in
	p)
	    PACKAGE=$OPTARG
	;;
	i)
	    SRC_DIR=$OPTARG
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

configure $PACKAGE $OS_V $SRC_DIR

