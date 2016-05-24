DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
OS_V=ubuntu-14.04-64
OUT_DIR=$DIR/../../out
SRC_DIR=$DIR/../../

PACKAGE=$1

PREFIX=$OUT_DIR/install/$OS_V/armadito-av

mkdir -p $PREFIX/var/lib/armadito/bases/
ln -s /var/lib/clamav $PREFIX/var/lib/armadito/bases/clamav
ln -s /var/lib/uhuru/bases/module5_2/ $PREFIX/var/lib/armadito/bases/module5_2
