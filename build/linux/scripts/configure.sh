
function configure()
{
	local PACKAGE_NAME=$1
	local SRC_DIR=$2
	local A_OPTS=""

	set -e
	mkdir -p $OUT_DIR/build/armadito-av/$PACKAGE_NAME

	set -e
	cd $SRC_DIR/
	chmod +x autogen.sh
	./autogen.sh	

	if [[ $PACKAGE_NAME == "prelude" ]];
	then
		A_OPTS="--with-libprelude-pythondir=$HOME/prelude/install/lib/python2.7/site-packages --with-libprelude-libdir=$HOME/prelude/install/lib"
	fi

	if [[ $PACKAGE_NAME == "core" ]];
	then
		A_OPTS='--enable-debug --enable-fanotify --enable-gcov'
	fi

	cd $OUT_DIR/build/armadito-av/$PACKAGE_NAME
	echo "-------CONFIGURE-------"
	$SRC_DIR/configure --prefix=$OUT_DIR/install/armadito-av PKG_CONFIG_PATH=$OUT_DIR/install/armadito-av/lib/pkgconfig $A_OPTS
	echo "-------CONFIGURE OKAY $PACKAGE_NAME-------"

}

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
OUT_DIR=$DIR/../out

function usage()
{
    echo "Usage: configure.sh -i SRC_DIR -p PACKAGE_NAME"
    echo "Configure package given before compiling"
    echo "Example: configure.sh -i /home/joebar/armadito-mod-clamav -p clamav"
    echo ""
    echo "Argument:"
    echo "  SRC_DIR              Sources directory"
    echo "  PACKAGE_NAME         Package name to configure"
    echo ""

    exit 1
}

while getopts "p:i:h" opt; do
    case $opt in
	p)
	    PACKAGE=$OPTARG
	;;
	i)
	    SRC_DIR=$OPTARG
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

configure $PACKAGE $SRC_DIR

