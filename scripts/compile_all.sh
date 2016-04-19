# Modify version here
OS_V=ubuntu-14.04-64

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
OUT_DIR=$DIR/../out
SRC_DIR=$DIR/../

PACKAGE=$1

set -e

if [[ $PACKAGE == "core" || $PACKAGE == "" ]];
then
	./configure.sh -p core -o $OS_V
	./compile.sh -p core -o $OS_V
fi

if [[ $PACKAGE == "clamav" || $PACKAGE == "" ]];
then
	./configure.sh -p modules/clamav -o $OS_V
	./compile.sh -p modules/clamav -o $OS_V
fi

if [[ $PACKAGE == "module5_2" || $PACKAGE == "" ]];
then
	./configure.sh -p modules/module5_2 -o $OS_V
	./compile.sh -p modules/module5_2 -o $OS_V
fi

#if [[ $PACKAGE == "modulePDF" || $PACKAGE == "" ]];
#then
	#./configure.sh -p modules/modulePDF -o $OS_V
#	./compile.sh -p modules/modulePDF -o $OS_V
#fi
