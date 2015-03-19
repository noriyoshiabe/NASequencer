#!/bin/bash

ROOT=$(cd $(dirname $0) && pwd)/..
xcodebuild -project $ROOT/cocoa/NAMIDI.xcodeproj -configuration Release clean build SYMROOT=$ROOT/bin/Build
mkdir $ROOT/release 2> /dev/null
cp -r $ROOT/bin/Build/Release/NAMIDI.app $ROOT/release/
rm -r $ROOT/bin/Build/Release/NAMIDI.app
