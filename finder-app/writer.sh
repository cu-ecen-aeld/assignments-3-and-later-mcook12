#!/bin/sh
# Script to create new file with given filename and path and content
# Author: Michael Cook

set -e

writefile=$1
writestr=$2

if [ $# -ne 2 ]; then
	echo "Error: Arguments not specified"
	exit 1
fi

dirstr=$(dirname ${writefile})

if [ ! -d "$dirstr" ]; then
	mkdir -p "$dirstr"
fi

if [ -d "$dirstr" ]; then
	echo "$writestr" > ${writefile}
fi
