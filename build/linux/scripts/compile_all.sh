# Modify version here
OS_V=ubuntu-14.04-64

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

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

if [[ $PACKAGE == "moduleH1" || $PACKAGE == "" ]];
then
	./configure.sh -p modules/moduleH1 -o $OS_V
	./compile.sh -p modules/moduleH1 -o $OS_V
fi

if [[ $PACKAGE == "modulePDF" || $PACKAGE == "" ]];
then
	./configure.sh -p modules/modulePDF -o $OS_V
	./compile.sh -p modules/modulePDF -o $OS_V
fi
