DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

PACKAGE=$1
REPO=$DIR
SCRIPTS=$REPO/core/build/linux/scripts

# Modify git repositories' paths here
CORE_SRC=$REPO/core
CLAMAV_SRC=$REPO/modules/mod-clamav
H1_SRC=$REPO/modules/mod-h1
PDF_SRC=$REPO/modules/mod-pdf
WEBUI_SRC=$REPO/ui/web-ui
PRELUDE_SRC=$REPO/agents/prelude/python
SYSTRAY_SRC=$REPO/ui/systray-ui/gtk

CONFIGURESH=$SCRIPTS/configure.sh
COMPILESH=$SCRIPTS/compile.sh

set -e

if [[ $PACKAGE == "core" || $PACKAGE == "" ]];
then
	. $CONFIGURESH -i $CORE_SRC -p core
	. $COMPILESH -p core
fi

if [[ $PACKAGE == "clamav" || $PACKAGE == "" ]];
then
	. $CONFIGURESH -i $CLAMAV_SRC -p clamav
	. $COMPILESH -p clamav
fi

if [[ $PACKAGE == "moduleH1" || $PACKAGE == "" ]];
then
	. $CONFIGURESH -i $H1_SRC -p moduleH1
	. $COMPILESH -p moduleH1
fi

if [[ $PACKAGE == "modulePDF" || $PACKAGE == "" ]];
then
	. $CONFIGURESH -i $PDF_SRC -p modulePDF
	. $COMPILESH -p modulePDF
fi

if [[ $PACKAGE == "python-prelude" ]];
then
	. $CONFIGURESH -i $PRELUDE_SRC -p prelude
	. $COMPILESH -p prelude
fi

if [[ $PACKAGE == "webui" || $PACKAGE == "" ]];
then
	. $CONFIGURESH -i $WEBUI_SRC -p webui
	. $COMPILESH -p webui
fi

if [[ $PACKAGE == "systray" || $PACKAGE == "" ]];
then
	. $CONFIGURESH -i $SYSTRAY_SRC -p systray
	. $COMPILESH -p systray
fi
