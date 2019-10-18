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
	echo "Please specify language code to generate/update"
fi

locale="$1"
podir="./po/$locale"
echo "Selected locale: $locale"
mkdir -p "$podir"
pots=`find . -name '*.pot'`
for pot in $pots; do
	po=`echo "$podir/${pot%.pot}.po" | sed -e "s/\/.\//\//g"`
	echo -n "$po: "
	
	if [ -f "$po" ]; then
		echo "update"
		msgmerge --update "$po" "$pot"
	else
		echo "generate"
		msginit --input="$pot" --locale="$locale" --output="$po"
	fi
	
done
