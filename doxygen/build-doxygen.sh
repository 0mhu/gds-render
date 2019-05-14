#!/bin/bash

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null && pwd )"
cd "$DIR"

export PROJECT_NUMBER=`../version/generate-version-string.sh`

if [ $# != 1 ]; then
	export OUTPUT_DIRECTORY="./output"
else
	export OUTPUT_DIRECTORY="$1"
fi

doxygen Doxyconfig
