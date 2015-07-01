set -x

. /etc/lsb-release 
DISTRIB_ID=$(echo ${DISTRIB_ID,,})
case $(uname -i) in
x86_64) DISTRIB_BITS=64 ;;
i386) DISTRIB_BITS=32 ;;
esac

PREFIX=$HOME/projects/uhuru/install/${DISTRIB_ID}-${DISTRIB_RELEASE}-${DISTRIB_BITS}
PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig qtcreator $* 
