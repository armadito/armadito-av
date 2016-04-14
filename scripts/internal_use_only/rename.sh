#!/bin/bash

for f in $* ; do
    echo "renaming in file $f"
    cp $f $f.BAK
    sed -e 's/uhuru_/a6o_/g' -e 's/UHURU/ARMADITO/g' < $f > $f.out
    mv $f.out $f
    sed -e 's/uhuru/armadito/g' < $f > $f.out
    mv $f.out $f
done
