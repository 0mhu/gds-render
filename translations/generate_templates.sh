#!/bin/bash

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null && pwd )"
cd "$DIR"

files=`find ../ -name "*.c"`
mkdir -p "pot"

for file in $files; do
	pot="pot/"$(echo "${file#*/}" | sed -e "s/\//_/g")
	pot="${pot%.c}.pot"	
	xgettext --keyword=_ --language=C --add-comments --sort-output -o "$pot" "$file"
done
