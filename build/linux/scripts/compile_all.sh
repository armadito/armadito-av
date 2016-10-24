DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

PACKAGE=$1
REPO=$DIR/../../../../

# Modify git repositories' paths here
CORE_SRC=$REPO/armadito-av
CLAMAV_SRC=$REPO/armadito-mod-clamav
H1_SRC=$REPO/armadito-mod-h1
PDF_SRC=$REPO/armadito-mod-pdf
WEBUI_SRC=$REPO/armadito-web-ui
PRELUDE_SRC=$REPO/armadito-prelude/python
SYSTRAY_SRC=$REPO/armadito-systray-ui/gtk

set -e

if [[ $PACKAGE == "core" || $PACKAGE == "" ]];
then
	./configure.sh -i $CORE_SRC -p core
	./compile.sh -p core
fi

if [[ $PACKAGE == "clamav" || $PACKAGE == "" ]];
then
	./configure.sh -i $CLAMAV_SRC -p clamav
	./compile.sh -p clamav
fi

if [[ $PACKAGE == "moduleH1" || $PACKAGE == "" ]];
then
	./configure.sh -i $H1_SRC -p moduleH1
	./compile.sh -p moduleH1
fi

if [[ $PACKAGE == "modulePDF" || $PACKAGE == "" ]];
then
	./configure.sh -i $PDF_SRC -p modulePDF
	./compile.sh -p modulePDF
fi

if [[ $PACKAGE == "python-prelude" ]];
then
	./configure.sh -i $PRELUDE_SRC -p prelude
	./compile.sh -p prelude
fi

if [[ $PACKAGE == "webui" || $PACKAGE == "" ]];
then
	./configure.sh -i $WEBUI_SRC -p webui
	./compile.sh -p webui
fi

if [[ $PACKAGE == "systray" || $PACKAGE == "" ]];
then
	./configure.sh -i $SYSTRAY_SRC -p systray
	./compile.sh -p systray
fi
