#!/bin/bash

ROOT=$(cd $(dirname $0) && pwd)/..
mkdir -p $ROOT/release/alpha 2> /dev/null
cp $ROOT/sample/example.namidi $ROOT/release/alpha/
cp $ROOT/sample/midi.trak $ROOT/release/alpha/
cp $ROOT/release/Applications $ROOT/release/alpha/
cp $ROOT/release/README.txt.alpha $ROOT/release/alpha/README.txt
cp -r $ROOT/release/NAMIDI.app $ROOT/release/alpha/

rm $ROOT/release/"NAMIDI alpha.dmg" 2> /dev/null
hdiutil create $ROOT/release/"NAMIDI alpha.dmg" -volname "NAMIDI Alpha" -fs HFS+ -srcfolder $ROOT/release/alpha
rm -r $ROOT/release/alpha
