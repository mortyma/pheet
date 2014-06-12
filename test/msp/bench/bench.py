#!/usr/bin/env python2

#Note: one may directly execute ./msp-bench. This python script only provides a more convenient user interface.
import argparse

REPETITIONS = 10

ALGORITHMS = [ 'sequential', 'strategy', 'strategy2' ]

# Number of cpus
# In the range [1, 80], contains:
#	- [1-8]
#	- all powers of 2
#	- all multiples of 5
CPUS = [ 1, 2, 3, 4, 5, 6, 7, 8, 10, 15, 16, 20, 25, 30, 32, 35, 40, 45, 50, 55, 60, 64, 65, 70, 75, 80 ]

MAX_CPUS = 80

if __name__ == '__main__':
	
	#parse arguments
	parser = argparse.ArgumentParser(
	   description='Benchmarking multi-criteria shortest path with different pheet task storage implementations.')
	parser.add_argument('-i','--input', help = 'Input file name (reuqired)', type = argparse.FileType('r'), required = True)
	parser.add_argument('-o','--output', help = 'Output file name (default: stdout)', type = argparse.FileType('w'), default = "stdout")
	parser.add_argument('-a', '--algorithm', help = "List of algorithms/variants to benchmark", nargs = "+", default = ALGORITHMS)
	parser.add_argument('-r', '--repetitions', help = "Number of repetitions", type = int, default = REPETITIONS)
	parser.add_argument('-n', '--nrcpus', help = "List of cpus counts", nargs = "+", type = int, default = CPUS)
	args = parser.parse_args()

	#make sure only correct algorithms/variants are given
	algorithms = args.algorithm
	for a in algorithms:
		if a not in ALGORITHMS:
			parser.error("Invalid algorithm: " + str(a))
			
	#make sure only valid cpu counts are given
	nrcpus = args.nrcpus
	for n in nrcpus:
		if n < 1 or n > MAX_CPUS:
			parser.error("Invalid number of cpus: " + str(n))