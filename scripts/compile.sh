function compile()
{
	local PACKAGE_NAME=$1
	local DISTRIB=$2

	cd $OUT_DIR/build/$OS_V/uhuru-av/$PACKAGE_NAME
	echo "-------MAKE-------"
	make 
	echo "-------MAKE INSTALL-------"
	make install
	echo "-------COMPILE OKAY $PACKAGE_NAME-------"

}

## Build pour Unbutu-14.04-64
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
OS_V=ubuntu-14.04-64

OUT_DIR=$DIR/../out
SRC_DIR=$DIR/../

function usage()
{
    echo "Usage: compile.sh -p PACKAGE -d DISTRIB"
    echo "Compile package given for a specific distrib"
    echo "Example: compile.sh -p modules/clamav -d trusty"
    echo ""
    echo "Argument:"
    echo "  PACKAGE          Package subpath to compile"
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

compile $PACKAGE $DISTRIB

