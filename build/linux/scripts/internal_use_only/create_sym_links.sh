DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
OUT_DIR=$DIR/../../out
SRC_DIR=$DIR/../../

PACKAGE=$1

PREFIX=$OUT_DIR/install/armadito-av

mkdir -p $PREFIX/var/lib/armadito/bases/
ln -s /var/lib/clamav $PREFIX/var/lib/armadito/bases/clamav
ln -s /var/lib/armadito/bases/moduleH1/ $PREFIX/var/lib/armadito/bases/moduleH1
