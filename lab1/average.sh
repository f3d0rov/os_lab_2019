#!/bin/bash 

sum=0
count=0
for num in "$@"
do
    sum=$((sum+num))
    count=$((count+1))
done

if [ $count -gt 0 ]
then
    echo $((100 * sum /count)) | sed 's/..$/.&/'
else 
    echo Expected user input
fi
