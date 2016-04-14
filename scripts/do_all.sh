
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
	#./package.sh -p core -d trusty
fi


if [[ $PACKAGE == "clamav" || $PACKAGE == "" ]];
then
	./configure.sh -p modules/clamav -d trusty
	./compile.sh -p modules/clamav -d trusty
	#./package.sh -p uhuru-mod-clamav -d trusty
fi

#if [[ $PACKAGE == "module5_2" || $PACKAGE == "" ]];
#then
	#./configure.sh -p modules/module5_2 -d trusty
	#./compile.sh -p modules/module5_2 -d trusty
	#./package.sh -p uhuru-mod-module5-2 -d trusty
#fi

if [[ $PACKAGE == "fanotify" || $PACKAGE == "" ]];
then
	./configure.sh -p modules/fanotify -d trusty
	./compile.sh -p modules/fanotify -d trusty
	#./package.sh -p uhuru-mod-fanotify -d trusty
fi

#if [[ $PACKAGE == "modulePDF" || $PACKAGE == "" ]];
#then
#	./configure.sh -p modules/modulePDF -d trusty
#	./compile.sh -p modules/modulePDF -d trusty
	#./package.sh -p uhuru-mod-fanotify -d trusty
#fi


## Old Interface Human Machine in QT
#if [[ $PACKAGE == "gui" || $PACKAGE == "" ]];
#then
#	./configure.sh -p gui -d trusty
#	./compile.sh -p gui -d trusty
	#./package.sh -p uhuru-qt -d trusty
#fi

## Old packaging calls
#if [[ $PACKAGE == "desktop" || $PACKAGE == "" ]];
#then
#	./package.sh -p uhuru-desktop -d trusty
#fi

#if [[ $PACKAGE == "cli" || $PACKAGE == "" ]];
#then
#	./package.sh -p uhuru-cli -d trusty
#fi

#if [[ $PACKAGE == "" ]];
#then
#	./update_repo.sh
#fi

