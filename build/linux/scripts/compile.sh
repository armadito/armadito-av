function compile()
{
	local PACKAGE_NAME=$1
	set -e

	cd $OUT_DIR/build/armadito-av/$PACKAGE_NAME
	echo "-------MAKE-------"
	make 
	echo "-------MAKE INSTALL-------"
	make install
	echo "-------COMPILE OKAY $PACKAGE_NAME-------"

	if [[ $PACKAGE_NAME == "webui" ]];
	then
		make bower
		make install-bower
	fi

	set +e
	make check
	echo "-------TESTS OKAY-------"
	make coverage
	echo "-------COVERAGE OKAY-------"
}

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
OUT_DIR=$DIR/../out

function usage()
{
    echo "Usage: compile.sh -p PACKAGE"
    echo "Compile given package"
    echo "Example: compile.sh -p modules/clamav"
    echo ""
    echo "Argument:"
    echo "  PACKAGE          Package subpath to compile"
    echo ""

    exit 1
}

while getopts "p:h" opt; do
    case $opt in
	p)
	    PACKAGE=$OPTARG
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

compile $PACKAGE

