#!/bin/sh
# Script to create new file with given filename and path and content
# Author: Michael Cook

writefile=$1
writestr=$2
dirstr=$(dirname ${writefile})

if [ $# -ne 2 ]; then
	echo "Error: Arguments not specified"
	exit 1
fi

if [ ! -d "$dirstr" ]; then
	mkdir -p "$dirstr"
fi

echo "$writestr" > ${writefile}


