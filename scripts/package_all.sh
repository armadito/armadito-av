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

set -e

cd $HOME/uhuru-linux-packaging

# Package all
./autogen.sh
./scripts/mktarball.sh -r $OUT_DIR
./configure --with-tarballdir=$OUT_DIR/sources
make -C packages/ubuntu/libuhuru package

# Install packages needed for dependances
sudo dpkg -i --force-all $(find packages/ubuntu/libuhuru/BUILD/ -iname "*.deb")

make -C packages/ubuntu package
make -C packages/ubuntu upload


