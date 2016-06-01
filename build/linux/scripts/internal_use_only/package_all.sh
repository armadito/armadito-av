DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
OS_V=ubuntu-14.04-64
OUT_DIR=$DIR/../../out
SRC_DIR=$DIR/../../

PACKAGE=$1

set -e

if [[ $PACKAGE == "core" || $PACKAGE == "" ]];
then
	./package.sh -p core -d trusty
fi

if [[ $PACKAGE == "clamav" || $PACKAGE == "" ]];
then
	./package.sh -p armadito-mod-clamav -d trusty
fi

if [[ $PACKAGE == "moduleH1" || $PACKAGE == "" ]];
then
	./package.sh -p armadito-mod-moduleH1 -d trusty
fi

#if [[ $PACKAGE == "modulePDF" || $PACKAGE == "" ]];
#then
	# TODO
	#./package.sh -p armadito-mod-modulepdf -d trusty
#fi

## Old packaging calls
#if [[ $PACKAGE == "desktop" || $PACKAGE == "" ]];
#then
#	./package.sh -p armadito-desktop -d trusty
#fi

#if [[ $PACKAGE == "cli" || $PACKAGE == "" ]];
#then
#	./package.sh -p armadito-cli -d trusty
#fi

#if [[ $PACKAGE == "" ]];
#then
#	./update_repo.sh
#fi

