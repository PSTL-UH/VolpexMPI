#!/bin/bash

for i in `seq 0  32` 




#	typesscript = `echo $i`

do
    outfilename=`echo $i.txt`
    quote=`echo '\\'`
    searchstring=`echo $quote[$i$quote]`
    grep $searchstring file > $outfilename
    wc -l $outfilename
done

i=-1
    outfilename=`echo txt.$i.txt`
    quote=`echo '\\'`
    searchstring=`echo $quote[$i$quote]`
    grep $searchstring file > $outfilename
    wc -l $outfilename

i=-2
    outfilename=`echo txt.$i.txt`
    quote=`echo '\\'`
    searchstring=`echo $quote[$i$quote]`
    grep $searchstring file > $outfilename
    wc -l $outfilename
