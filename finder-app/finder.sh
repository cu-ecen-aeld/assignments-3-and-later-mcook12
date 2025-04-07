#!/bin/bash
# Script to find files in given path that contain given text
# Author: Michael Cook

set -e

filenum=$(grep -r -l $2 $1 | wc -l)
linenum=$(grep -r $2 $1 | wc -l)

echo "The number of files are ${filenum} and the number of matching lines are ${linenum}"

if [ $# -ne 2 ]; then
	echo "Error: Parameters not specified"
	exit 1
fi

if [ ! -d $1 ]; then
	echo "Error: Directory does not Exist"
	exit 1
fi

