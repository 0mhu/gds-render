#!/bin/bash

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null && pwd )"
cd "$DIR"

if [ -z $1 ]; then
	echo "Must supply an output name"
	exit -2
fi

for langdir in `find ./pot/po -mindepth 1 -maxdepth 1 -type d`; do
	lang=`basename "$langdir"`
	dest="$1/locale/$lang/LC_MESSAGES"
	mkdir -p "$dest"
	pofiles=`find "$langdir" -name "*.po" | tr '\n' ' '`
	comb=`msgcat $pofiles`
	echo "$comb" | msgfmt --output-file="$dest/gds-render.mo" -
done
