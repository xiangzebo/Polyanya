#!/bin/bash
stamp=`date "+%d/%m/%y %H:%M:%S"`" $1"
echo "$stamp $0"
echo "$stamp $0" >> log

function help
{
	echo "Syntax: ./$0 \"[path to scenario files]\" \"[args to pass hog]\" [number of runs]"
	echo "Example: ./$0 \"experiments/scenarios/local/*\" \"-search jps\" 3"
	echo "NB:"
	echo "Quotation marks are important. Don't omit them!"
	echo "It may also be necessary to append the wildcard character * to the end of "
	echo "the path if every file in the target directory is a scenario"
}

if [ $# -le 1 ]
then
	echo "Error: Invalid syntax. $0 $1 $2 $3"
	help
	exit 1
fi

runs=
if [ -n "$3" ]
then
	runs=$3
else
	runs=1
fi

for ((x=1;x<=$runs;x++)) ; do
	for i in `ls $1/*.map.scen`;
	do
			sfile=$i
			echo $sfile
			./bin/warthog --scen $sfile $2 

			if [ "$?" -ne "0" ]
			then
				err="Failed while executing: $0 --scen $sfile $2";
				echo $err >> log
				echo $err
				exit 1
			fi
	done
done

