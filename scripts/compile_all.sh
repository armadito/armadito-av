DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
OS_V=ubuntu-14.04-64
OUT_DIR=$DIR/../out
SRC_DIR=$DIR/../

PACKAGE=$1

set -e

if [[ $PACKAGE == "core" || $PACKAGE == "" ]];
then
	./configure.sh -p core -d trusty
	./compile.sh -p core -d trusty
fi

if [[ $PACKAGE == "clamav" || $PACKAGE == "" ]];
then
	./configure.sh -p modules/clamav -d trusty
	./compile.sh -p modules/clamav -d trusty
fi

if [[ $PACKAGE == "module5_2" || $PACKAGE == "" ]];
then
	./configure.sh -p modules/module5_2 -d trusty
	./compile.sh -p modules/module5_2 -d trusty
fi

#if [[ $PACKAGE == "modulePDF" || $PACKAGE == "" ]];
#then
	#./configure.sh -p modules/modulePDF -d trusty
#	./compile.sh -p modules/modulePDF -d trusty
#fi
