## Build pour Unbutu-14.04-64
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
OS_V=ubuntu-14.04-64

OUT_DIR=$DIR/../out
SRC_DIR=$DIR/../

mkdir -p $OUT_DIR/build/$OS_V/uhuru-av/libuhuru/pkgconfig
mkdir -p $OUT_DIR/build/$OS_V/uhuru-av/gui
mkdir -p $OUT_DIR/build/$OS_V/uhuru-av/modules/module5_2
mkdir -p $OUT_DIR/build/$OS_V/uhuru-av/modules/clamav

set -e

#cd $SRC_DIR
#sudo chown -R uhuru:uhuru .

cd $SRC_DIR/libuhuru/
sudo chmod +x autogen.sh
./autogen.sh	

cd $OUT_DIR/build/$OS_V/uhuru-av/libuhuru
echo "-------CONFIGURE-------"
$SRC_DIR/libuhuru/configure --prefix=$OUT_DIR/install/$OS_V/uhuru-av
echo "-------OKAY LIBUHURU-------"

cd $SRC_DIR/modules/clamav/
sudo chmod +x autogen.sh
./autogen.sh	

cd $OUT_DIR/build/$OS_V/uhuru-av/modules/clamav
echo "-------CONFIGURE-------"	
$SRC_DIR/modules/clamav/configure --prefix=$OUT_DIR/install/$OS_V/uhuru-av
echo "-------OKAY CLAMAV-------"

cd $SRC_DIR/modules/module5_2/
sudo chmod +x autogen.sh
./autogen.sh	

cd $OUT_DIR/build/$OS_V/uhuru-av/modules/module5_2
echo "-------CONFIGURE-------"
$SRC_DIR/modules/module5_2/configure --prefix=$OUT_DIR/install/$OS_V/uhuru-av
echo "-------OKAY MODULE5_2-------"

cd $SRC_DIR/gui/
sudo chmod +x autogen.sh
./autogen.sh	

cd $OUT_DIR/build/$OS_V/uhuru-av/gui
echo "-------CONFIGURE-------"
$SRC_DIR/gui/configure --prefix=$OUT_DIR/install/$OS_V/uhuru-av
echo "-------OKAY QTGUI-------"

## On lui indique où trouver les bases
set +e
mkdir -p $OUT_DIR/install/$OS_V/uhuru-av/var/lib/uhuru/
ln -s /var/lib/uhuru/bases/ $OUT_DIR/install/$OS_V/uhuru-av/var/lib/uhuru/bases


