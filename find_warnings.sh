#!/bin/bash
gcc -Wall $1 -o test 2> warning.txt
sed  's/warning: //g' warning.txt > result.txt
gcc $1 -o test
if [ $? -eq 0 ]
then
		a=1
		while [ $a -le $2 ]
		do
				echo $a | echo `./test` >> result.txt
				a=$[$a+1]
		done
fi
pwd >> result.txt	
