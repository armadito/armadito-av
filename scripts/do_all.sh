
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
OS_V=ubuntu-14.04-64
OUT_DIR=$DIR/../out
SRC_DIR=$DIR/../

PACKAGE=$1

if [[ $PACKAGE == "libuhuru" || $PACKAGE == "" ]];
then
	./configure.sh -p libuhuru -d trusty
	./compile.sh -p libuhuru -d trusty
	./package.sh -p libuhuru -d trusty
fi

if [[ $PACKAGE == "gui" || $PACKAGE == "" ]];
then
	./configure.sh -p gui -d trusty
	./compile.sh -p gui -d trusty
	./package.sh -p uhuru-qt -d trusty
fi

if [[ $PACKAGE == "clamav" || $PACKAGE == "" ]];
then
	./configure.sh -p modules/clamav -d trusty
	./compile.sh -p modules/clamav -d trusty
	./package.sh -p uhuru-mod-clamav -d trusty
fi

if [[ $PACKAGE == "module5_2" || $PACKAGE == "" ]];
then
	./configure.sh -p modules/module5_2 -d trusty
	./compile.sh -p modules/module5_2 -d trusty
	./package.sh -p uhuru-mod-module5-2 -d trusty
fi

if [[ $PACKAGE == "fanotify" || $PACKAGE == "" ]];
then
	./configure.sh -p modules/fanotify -d trusty
	./compile.sh -p modules/fanotify -d trusty
	./package.sh -p uhuru-mod-fanotify -d trusty
fi

if [[ $PACKAGE == "desktop" || $PACKAGE == "" ]];
then
	./package.sh -p uhuru-desktop -d trusty
fi

if [[ $PACKAGE == "cli" || $PACKAGE == "" ]];
then
	./package.sh -p uhuru-cli -d trusty
fi

if [[ $PACKAGE == "" ]];
then
	./update_repo.sh
fi

