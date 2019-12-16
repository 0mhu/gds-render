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

# C Files

pot="pot/gds-render.pot"

for file in $files; do
	echo "Parsing C file $file"
	# pot="pot/"$(echo "${file#*/}" | sed -e "s/\//_/g")
	# pot="${pot%.c}.pot"
	if [[ -f "$pot" ]]; then
		xgettext --package-name="gds-render" --join-existing --keyword=_ --language=C --add-comments --sort-output -o "$pot" "$file"
	else
		xgettext --package-name="gds-render" --keyword=_ --language=C --add-comments --sort-output -o "$pot" "$file"
	fi
done

# Glade files
glade_files=`find ../resources/ -name "*.glade"`
for glade in $glade_files; do
	echo "Parsing Glade file $glade"
	if [[ -f "$pot" ]]; then
		xgettext --package-name="gds-render" --join-existing --keyword=_ -L Glade --sort-output -o "$pot" "$glade"
	else
		xgettext --package-name="gds-render" --keyword=_ -L Glade --sort-output -o "$pot" "$glade"
	fi
done
