
# Quick Start 

mkdir build
cd build
make
make install

# Run 

./lhe_test -t 0 -d 5

./cmp_main -n 16 -m 2 -c 0

./pdte_main -i ../data/heart_11bits -n 11 -m 11 -d 10 -p 0

./pdte_main -i ../data/heart_11bits -n 11 -m 11 -d 10 -p 1

./bpdte_main -i ../data/heart_11bits -d 128 -n 11 -m 6

./bpdte_main -i ../data/heart_11bits -d 1024 -n 11 -m 11 -c 1

./bpdte_main -i ../data/heart_11bits -d 16384 -n 11 -m 1 -c 1

./bpdte_main -i ../data/heart_11bits -d 16384 -n 11 -m 4 -c 2

# Parameter Settings 

1.  -i  [Path]
        The directory path containing the decision tree model and the dataset.

2.  -o  [Path]
        The directory path to save the decision tree model and the dataset.
        
3.  -d  [Batch Size]
        The number of input data rows for batch evaluation.

4.  -n  [Bit Precision]
        The total bit length  n.

5.  -m  [Encoding Length]
        Controls the encoding granularity in CMP; 
        The number of ciphertexts is l = n / m.
        In TCMP, 2^m slot per elements in a LHE ciphertext.
        In BCMP, m slot per elements in a LHE ciphertext.

6.  -h  [Hamming Weight]
        Controls the encoding granularity in CWCMP;
        The number of ciphertexts is l where (l h) > 2^n.
        In CWCMP, one slot per elements in a LHE ciphertext.

7.  -c  [CMP Mode]
        Select the comparison algorithm:
        0: TCMP (Thermometer Comparison)
        1: DCMP (Dichotomy Comparison)
        2: CWCMP (https://eprint.iacr.org/2024/662)

8.  -e  [Reserved Depth]
        The extra multiplicative depth reserved after the comparison step.

9.  -p  [PDTE Evaluation Strategy]
        Select the private decision tree evaluation (PDTE) method:
        0: PDTE with SumPath
        1: PDTE with SumPath2
        3: PDTE with MultiPath
        Select the batch PDTE method:
        0: BPDTE with ASM (Adapted SumPath)
        1: BPDTE with ESM (Extended SumPath)

10.  -t  [LHE Scheme Type]
        Select the underlying homomorphic encryption scheme:
        0: BFV
        1: BGV

# Benchmark 

For detailed instructions on running the performance benchmarks, 
please refer to the execution sequences documented in:
./benchmark_cmp.sh
./benchmark_pdte.sh
./benchmark_bpdte.sh

This script contains the automated terminal commands required to evaluate the throughput and latency of the private comparison and private decison tree evaluation system.

# Comparison Protocols 

Detailed terminal commands and environment setup for the 
baseline comparison protocols can be found in the following 
sub-repositories:

1.  Level Up Comparison: (https://arxiv.org/abs/2309.06496)
    Location: ./extern_respositories/level_up/Readme.md

2.  SortingHat Comparison: (https://eprint.iacr.org/2022/757)
    Location: ./extern_respositories/sortinghat/Readme.md

3.  BPDTE_CW Comparison: (https://eprint.iacr.org/2024/662)
    Location: ./src/cwcmp.cc