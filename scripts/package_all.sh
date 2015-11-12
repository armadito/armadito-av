DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
OS_V=ubuntu-14.04-64
PACKAGE_VERSION=1.5.1 

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

FULL_VERSION=$PACKAGE_VERSION-0ubuntu1+trusty1

# Package all
./autogen.sh
./scripts/mktarball.sh -r $OUT_DIR
./configure --with-tarballdir=$OUT_DIR/sources
make -C packages/ubuntu/libuhuru package

# Install packages needed fcr dependances
sudo dpkg -i --force-all packages/ubuntu/libuhuru/BUILD/libuhuru_"$FULL_VERSION"_amd64.deb 
sudo dpkg -i --force-all packages/ubuntu/libuhuru/BUILD/libuhuru-dev_"$FULL_VERSION"_amd64.deb 
sudo dpkg -i --force-all packages/ubuntu/libuhuru/BUILD/libuhuru-tools_"$FULL_VERSION"_amd64.deb

make -C packages/ubuntu package
make -C packages/ubuntu upload


