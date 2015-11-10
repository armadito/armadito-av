## Build pour Unbutu-14.04-64
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
OS_V=ubuntu-14.04-64

OUT_DIR=$DIR/../out
SRC_DIR=$DIR/../

set -e

cd $OUT_DIR/build/$OS_V/uhuru-av/libuhuru
echo "-------MAKE-------"
make 
echo "-------MAKE INSTALL-------"
make install
echo "-------OKAY LIBUHURU-------"

# On copie libuhuru.pc afin que pkg-config puisse trouver le paquet LIBUHURU lors de la compilation du module clamav
## Attention, à tester sur d'autres OS. Le dossier inclus par défaut n'est pas forcément /usr/lib/pkgconfig.
## Alternative : utiliser PKG_CONFIG_PATH
sudo cp -fv $OUT_DIR/build/$OS_V/uhuru-av/libuhuru/pkgconfig/*.pc /usr/lib/pkgconfig

cd $OUT_DIR/build/$OS_V/uhuru-av/modules/clamav
echo "-------MAKE-------"
make 
echo "-------MAKE INSTALL-------"
make install
echo "-------OKAY CLAMAV-------"

cd $OUT_DIR/build/$OS_V/uhuru-av/modules/module5_2
echo "-------MAKE-------"
make 
echo "-------MAKE INSTALL-------"
make install
echo "-------OKAY MODULE5_2-------"

cd $OUT_DIR/build/$OS_V/uhuru-av/gui
echo "-------MAKE-------"
make 
echo "-------MAKE INSTALL-------"
make install
echo "-------OKAY QTGUI-------"

