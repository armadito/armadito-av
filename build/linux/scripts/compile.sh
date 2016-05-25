function compile()
{
	local PACKAGE_NAME=$1
	local OS_V=$2

	set -e

	cd $OUT_DIR/build/$OS_V/armadito-av/$PACKAGE_NAME
	echo "-------MAKE-------"
	make 
	echo "-------MAKE INSTALL-------"
	make install
	echo "-------COMPILE OKAY $PACKAGE_NAME-------"

}

## Build pour Unbutu-14.04-64
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

OUT_DIR=$DIR/../out
SRC_DIR=$DIR/../../../

function usage()
{
    echo "Usage: compile.sh -p PACKAGE -o OS_V"
    echo "Compile package given for a specific OS version"
    echo "Example: compile.sh -p modules/clamav -o ubuntu-14.04-64"
    echo ""
    echo "Argument:"
    echo "  PACKAGE          Package subpath to compile"
    echo "  OS_V          OS version out subdirectory name"
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

compile $PACKAGE $OS_V

