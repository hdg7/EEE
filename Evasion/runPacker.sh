#!/bin/bash

#####################
# $1 Types Number
# $2 ConfigFile Values
####################

delay=$(echo $2 | cut -f1 -d',')
B0=$(echo $2 | cut -f2-9 -d',' | tr ',' '\n' )
delimiter=$( for i in `echo $B0` ; do printf \\$(printf "%o" $i);done;)
B1=$(echo $2 | cut -f10-17 -d',')
B2=$(echo $2 | cut -f18-22 -d',')
A=$(echo $2 | cut -f23- -d',' | tr ',' ' ' )
echo "[Bubbles]"
echo "total=$1"
echo "delimiter=$delimiter"
echo "delay=$delay"
echo "order=$B1"
echo "orderDel=$B2"
counter=0
for word in $A
do
	j=$((counter / 5))
	i=$((counter % 5))
	case $i in
	0)
		echo "[bubble$j]"
		echo "Number=$word"
	;;
	1)
		echo "Median=$word"
	;;
	2)
		echo "Deviation=$word"
	;;
	3)
		echo "Size=$word"
	;;
	4)
		echo "Density=$word"
	;;
	esac
	counter=$((counter + 1))
done
