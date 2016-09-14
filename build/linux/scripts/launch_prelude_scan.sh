DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
OUT_DIR=$DIR/../out
SRC_DIR=$DIR/../../../
set -e

PREFIX=$OUT_DIR/install/armadito-av
$PREFIX/bin/armadito-prelude-agent -a scan $1
