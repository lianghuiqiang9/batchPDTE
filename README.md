
# Quick Start 

mkdir build
cd build
make
make install

# Run 

./lhe_test -t 0 -d 5

./cmp_main -m 8 -l 2 -n 16 -t 0

./tree_test -i ../data/heart_11bits -o ../data/heart_11bits_temp -d 16

./bpdte_main -i ../data/heart_11bits -d 2048 -l 8 -m 2 -n 16 -c 0 -e 0 -p 0

./bpdte_main -i ../data/heart_11bits -d 1024 -l 8 -m 2 -n 16 -c 0 -e 5 -p 1

./pdte_main -i ../data/heart_11bits -l 1 -m 16 -d 10

# Parameter Settings 

1.  -i  [Path]
        The directory path containing the decision tree model and the dataset.

2.  -d  [Batch Size]
        The number of input data rows for batch evaluation.

3.  -l  [TE Expansion Factor]
        Controls the number of ciphertexts in TEcmp; 
        The total bit length is (l * m).

4.  -m  [TE Encoding Length]
        Controls the encoding granularity in TEcmp; 
        The total bit length is (l * m).

6.  -c  [Comparison Mode]
        Select the comparison algorithm:
        0: tCMP (Thermometer Comparison)
        1: dCMP (Dichotomy Comparison)

7.  -e  [Reserved Depth]
        The extra multiplicative depth reserved after the comparison step.

8.  -p  [Evaluation Strategy]
        Select the Private Decision Tree Evaluation (bpdte) method:
        0: bPDTE with ASM (Adapted Sum Path)
        1: bPDTE with ESM (Extended Sum Path)

9.  -t  [Scheme Type]
        Select the underlying Homomorphic Encryption scheme:
        0: BFV
        1: BGV


# Benchmark 

For detailed instructions on running the performance benchmarks, 
please refer to the execution sequences documented in:
./benchmark.sh

This script contains the automated terminal commands required to 
evaluate the throughput and latency of the privCMP and bpdte system.


# Comparison Protocols 

Detailed terminal commands and environment setup for the 
baseline comparison protocols can be found in the following 
sub-repositories:

1.  Level Up Comparison:
    Location: ./extern_respositories/level_up/Readme.md

2.  SortingHat Comparison:
    Location: ./extern_respositories/sortinghat/Readme.md

