function increment_version()
{
	local PKG_PATH=$1

	cd $HOME/uhuru-linux-packaging/$PKG_PATH/debian
	#dch --increment --distribution $DISTRIB 'Auto-increment' --controlmaint --force-distribution --upstream

	debchange --increment --distribution trusty --controlmaint "Fixing package dependencies"
	cd $HOME/uhuru-linux-packaging
}

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
OS_V=ubuntu-14.04-64
OUT_DIR=$DIR/../out
SRC_DIR=$DIR/../

TOP_BUILDDIR=$OUT_DIR/build/$OS_V
TOP_SOURCEDIR=$OUT_DIR/sources
TOP_PKGBUILDDIR=$OUT_DIR/build/packages/$OS_V
export GNUPGHOME=$HOME/uhuru-linux-packaging/gpg

mkdir -p $TOP_SOURCEDIR
mkdir -p $TOP_PKGBUILDDIR

function usage()
{
    echo "Usage: package.sh -p PACKAGE -d DISTRIB"
    echo "Generate specified package for a specific distrib"
    echo "Example: package.sh -p modules/clamav -d trusty"
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

set -e

cd $HOME/uhuru-linux-packaging

# Package all
./autogen.sh
./scripts/mktarball.sh -r $OUT_DIR
./configure --with-tarballdir=$OUT_DIR/sources

# Clean BUILD dir and install packages needed for dependances
if [[ $PACKAGE == "core" ]];
then 
	rm -r packages/ubuntu/libarmadito/BUILD/ 
	make -C packages/ubuntu/libarmadito package


	# We need to temporarly install libuhuru packages
	sudo dpkg -i $(find . -iname libarmadito_*.deb)
	sudo dpkg -i $(find . -iname libarmadito-dev_*.deb)
	sudo dpkg -i $(find . -iname libarmadito-tools_*.deb)

	#increment_version packages/ubuntu/libuhuru
	make -C packages/ubuntu/libarmadito upload

	echo "-------PACKAGING OKAY $PACKAGE-------"
	exit 0
else
	make -C packages/ubuntu/$PACKAGE package
	#increment_version packages/ubuntu/$PACKAGE
	make -C packages/ubuntu/$PACKAGE upload
	echo "-------PACKAGING OKAY $PACKAGE-------"
fi


