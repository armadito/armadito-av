## Old Qt gui (linux only)

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
OS_V=ubuntu-14.04-64
OUT_DIR=$DIR/../../out
SRC_DIR=$DIR/../../

set -e

./configure.sh -p gui -d trusty
./compile.sh -p gui -d trusty
./package.sh -p uhuru-qt -d trusty

