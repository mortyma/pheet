#!/usr/bin/env python2

#Note: one may directly execute ./msp-bench. This python script only provides a more convenient user interface.

import argparse
import os
import subprocess 
from  subprocess import Popen, PIPE, STDOUT
import sys


REPETITIONS = 10

ALGORITHMS = [ 'sequential', 'strategy', 'strategy2' ]

# Number of cpus
# In the range [1, 80], contains:
#    - [1-8]
#    - all powers of 2
#    - all multiples of 5
CPUS = [ 1, 2, 3, 4, 5, 6, 7, 8, 10, 15, 16, 20, 25, 30, 32, 35, 40, 45, 50, 55, 60, 64, 65, 70, 75, 80 ]

MAX_CPUS = 80

def bench(algorithms, nrcpus, repetitions, inFile, outDir):
    #make sure only correct algorithms/variants are given
    for a in algorithms:
        if a not in ALGORITHMS:
            parser.error("Invalid algorithm: " + str(a))
            
    #make sure only valid cpu counts are given
    for n in nrcpus:
        if n < 1 or n > MAX_CPUS:
            parser.error("Invalid number of cpus: " + str(n))
    
    #assemble the argument/option string for ./msp-bench
    call = ["./build/test/msp/bench/msp-bench"]
    call += [inFile.name]
    call += ["-r " + str(repetitions)]
  
    for n in nrcpus:
        call += ["-n " + str(n)]
        
    #run ./msp-bench  as a subprocess
    for a in algorithms:
        ret = subprocess.check_output(call + ["--" + a])
        # parse benchmark data to more regular csv format
        p = Popen(['./csvheet'], stdout=PIPE, stdin=PIPE, stderr=PIPE)
        stdout_data = p.communicate(input=ret)[0]
        if outDir:
            outFile = os.path.join(outDir, a + ".dat")
            f = open(outFile, 'w')
            f.write(stdout_data)
        else:
            print stdout_data

if __name__ == '__main__':
    #make
    subprocess.call(["make", "release"])
    
    #parse arguments
    parser = argparse.ArgumentParser(
       description='Benchmarking multi-criteria shortest path with different pheet task storage implementations.')
    parser.add_argument('-i','--input', help = 'Input file name (reuqired)', type = argparse.FileType('r'), required = True)
    parser.add_argument('-o','--output', help = 'Output directory. If given, "directory/<algorithm>.dat" will be used to save results for each algorithm given by -a. Otherwise, output is written to stdout', action = 'store')
    parser.add_argument('-l','--limit', help = 'Benchmark sequential and strategy', action='store_true')
    parser.add_argument('-s', '--small', help = 'Run for 1, 2 and 4 cpus only', action='store_true')
    parser.add_argument('-a', '--algorithm', help = "List of algorithms/variants to benchmark. Available/default: {sequential, strategy, strategy2}", nargs = "+", default = ALGORITHMS)
    parser.add_argument('-r', '--repetitions', help = "Number of repetitions", type = int, default = REPETITIONS)
    parser.add_argument('-n', '--nrcpus', help = "List of cpus counts", nargs = "+", type = int, default = CPUS)
    args = parser.parse_args()
    
    if args.output:   
        if os.path.isfile(args.output):
            raise ValueError("-o needs to specify a directory")
        if not os.path.exists(args.output):
            os.makedirs(args.output)
    
    if args.limit:
        algorithms = ["sequential", "strategy"]
    else:
        algorithms = args.algorithm
    if args.small:
        nrcpus = [1,2,3,4]
    else:
        nrcpus = args.nrcpus
    
    bench(algorithms, nrcpus, args.repetitions, args.input, args.output)

    