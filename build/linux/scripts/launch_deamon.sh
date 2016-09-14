DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

OUT_DIR=$DIR/../out
SRC_DIR=$DIR/../../../
set -e

PREFIX=$OUT_DIR/install/armadito-av
G_MESSAGES_DEBUG=all LD_LIBRARY_PATH=$PREFIX/lib PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig $PREFIX/sbin/armadito-scand --no-daemon --log-level=debug

# -- using valgrind
#G_MESSAGES_DEBUG=all LD_LIBRARY_PATH=$PREFIX/lib PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig valgrind --leak-check=full --show-leak-kinds=definite --log-file="a6o_leaks.log" $PREFIX/sbin/armadito-scand --no-daemon --log-level=debug
