#!/bin/bash

tmp_file=$1.tmp
IFS=''
CLASS_found=0

while read line; 
do

if [ "x$line" == 'x[" CLASS "]' ] ; then 
 CLASS_found=1
  	
fi
if [ ${CLASS_found} -eq 1 -a "x$line" != "x#" ]; then
	continue
fi
CLASS_found=0

echo $line >> $tmp_file 

done < $1

mv $tmp_file $1
