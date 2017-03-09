DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

PACKAGE_NAME=$1
if [ -z "$PACKAGE_NAME" ]
then
	PACKAGE_NAME="ALL"
fi

REPO=$DIR
SCRIPTS=$REPO/armadito-core/build/linux/scripts

# Modify git repositories' paths here
CORE_SRC=$REPO/armadito-core
CLAMAV_SRC=$REPO/modules/mod-clamav
H1_SRC=$REPO/modules/mod-h1
PDF_SRC=$REPO/modules/mod-pdf
WEBUI_SRC=$REPO/ui/web-ui
SYSTRAY_SRC=$REPO/ui/systray-ui/gtk

CONFIGURESH=$SCRIPTS/configure.sh
COMPILESH=$SCRIPTS/compile.sh

set -e

if [[ $PACKAGE_NAME == "core" || $PACKAGE_NAME == "ALL" ]];
then
	. $CONFIGURESH -i $CORE_SRC -p core
	. $COMPILESH -p core
fi

if [[ $PACKAGE_NAME == "clamav" || $PACKAGE_NAME == "ALL" ]];
then
	. $CONFIGURESH -i $CLAMAV_SRC -p clamav
	. $COMPILESH -p clamav
fi

if [[ $PACKAGE_NAME == "moduleH1" || $PACKAGE_NAME == "ALL" ]];
then
	. $CONFIGURESH -i $H1_SRC -p moduleH1
	. $COMPILESH -p moduleH1
fi

if [[ $PACKAGE_NAME == "modulePDF" || $PACKAGE_NAME == "ALL" ]];
then
	. $CONFIGURESH -i $PDF_SRC -p modulePDF
	. $COMPILESH -p modulePDF
fi

if [[ $PACKAGE_NAME == "webui" || $PACKAGE_NAME == "ALL" ]];
then
	. $CONFIGURESH -i $WEBUI_SRC -p webui
	. $COMPILESH -p webui
fi

if [[ $PACKAGE_NAME == "systray" || $PACKAGE_NAME == "ALL" ]];
then
	. $CONFIGURESH -i $SYSTRAY_SRC -p systray
	. $COMPILESH -p systray
fi
