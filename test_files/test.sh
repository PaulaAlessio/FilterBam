#!/bin/bash 

inputfile=example.bam
filter=filter.fai
prefix="HWI-ST933:227:C8C7YACXX:1:"
output1=wo_secondary.sam
output2=with_secondary.sam

../bin/FilterBam -i $inputfile -f $filter -p $prefix  -o $output1
../bin/FilterBam -i $inputfile -f $filter -p $prefix  -o $output2 -s

