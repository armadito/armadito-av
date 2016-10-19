DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

PACKAGE=$1
REPO=$DIR/../../../../

# Modify git repositories' paths here
CORE_SRC=$REPO/armadito-av
CLAMAV_SRC=$REPO/armadito-mod-clamav
H1_SRC=$REPO/armadito-mod-h1
PDF_SRC=$REPO/armadito-mod-pdf
GUI_SRC=$REPO/armadito-gui/web
PRELUDE_SRC=$REPO/armadito-prelude/python

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

if [[ $PACKAGE == "gui" || $PACKAGE == "" ]];
then
	./configure.sh -i $GUI_SRC -p gui
	./compile.sh -p gui
fi
