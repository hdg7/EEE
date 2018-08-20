#!/bin/bash

#$1 variant
#$2 configFile

NAME=$(basename $1)
CONFIGFILE=$(echo $2 | cut -f1 -d',')
SEGMENTS=$(echo $2 | cut -f2 -d',')
#cp $CONFIGFILE .
/home/menendez/projects/EntCalculator/entCalculator -i $1 -f $CONFIGFILE
/home/menendez/projects/EntCalculator/entCalculator -i segmentation$NAME -d $SEGMENTS/ > /tmp/b.$$.txt
echo "$1;0;0;0" > /tmp/a.$$.txt
if [ "$#" -eq 3 ]; then
	paste /tmp/a.$$.txt /tmp/b.$$.txt -d';' > $NAME.$3.input
else
	paste /tmp/a.$$.txt /tmp/b.$$.txt -d';' > $NAME.input
fi
rm /tmp/a.$$.txt /tmp/b.$$.txt
