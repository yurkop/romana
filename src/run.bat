#!/bin/bash
for ((i=0; i<=4; i++ ))
do
for ((j=0; j<=9; j++ ))
do
for ((k=-5; k<=5; k++ ))
do
for ((l=-5; l<=5; l++ ))
do

#echo $i $j
rootfile=res\_$i\_$j\_$k\_$l.root

romana.x file.dat -p par.par sTg=$i sS=$j T1=$k T2=$l

/bin/mv res.root $rootfile
echo $i $j $k $l $rootfile

done
done
done
done
