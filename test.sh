#!/bin/bash

times_ans=0
perf_ans=0

####################################################
################## GET INPUT #######################
####################################################
while true; do
	read -p "Do you wish to run timing tests? " ans
	case $ans in
		[Yy]* ) let times_ans=1; break;;
		[Nn]* ) break;;
		* ) echo "Please answer y/N.";;
	esac
done

while true; do
	read -p "Do you wish to run perf tests? " ans
	case $ans in
		[Yy]* ) let perf_ans=1; break;;
		[Nn]* ) break;;
		* ) echo "Please answer y/N.";;
	esac
done

valid_ans=0
while true; do
	read -p "Do you wish to run validity tests? " ans
	case $ans in
		[Yy]* ) let valid_ans=1; break;;
		[Nn]* ) break;;
		* ) echo "Please answer y/N.";;
	esac
done


#################################################
################# COMPILATION ###################
#################################################
make clean
make pagerank


################################################
################### TIME TEST ##################
################################################
if [ $times_ans -eq 1 ]
then
	echo "-------------------- TIME TESTS --------------------"

	# Remove prevous output
	rm -f out.txt

	for f in test/tests/*.in 
	# For every file in the test time block loop through them an
	do
		let len=${#f}-2
		fname=${f:11:len}
		echo "Test: $fname"
		time ./pagerank < $f >> out.txt
	done
fi


################################################
################# PERF TEST ####################
################################################
if [ $perf_ans -eq 1 ]
then
	echo "-------------------- PERF TESTS --------------------"
	sudo perf record -F 100 -a -g -- ./pagerank < test/tests/test12.in
	sudo perf script > "out/perf/out.perf"
	./stackcollapse-perf.pl "out/perf/out.perf" > "out/folded/out.folded"
	./flamegraph.pl --hash "out/folded/out.folded" > "out/svg/out.svg"
fi


################################################
############### VALIDITY TESTS #################
################################################
if [ $valid_ans -eq 1 ]
then
	echo "-------------------- VALIDITY TESTS ----------------"
	echo "Testing Non-Parallel."
	for f in test/tests/*.in
	do
		let len=${#f}-2
		fname=${f:0:len}out
		echo "Test $fname"
		./pagerank < $f | diff - $fname
	done
	
	echo "Testing Parallel Implementation."
	for f in test/tests/*.in
	do
		let len=${#f}-2
		fname=${f:0:len}out
		echo "Test $fname"
		./pagerank < $f | diff - $fname
	done
fi

################################################
################# CLEAN UP #####################
################################################
make clean
rm -f perf.data
