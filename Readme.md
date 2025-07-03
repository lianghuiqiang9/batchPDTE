# Benckmark
The data presented in Figure 5, Figure 6, and Table 8 was obtained through tests described in benchmark.md.



# Main
Testing Our Program

# Install libcmp.so
cd CMP
bash installCMP.sh
cd ..

# cmp_branch

cd cmp_bench
bash build_cmp_bench.sh
cd build

./tecmp -l 4 -m 2
./tecmp -l 8 -m 2
./tecmp -l 16 -m 2
./tecmp -l 32 -m 2
./tecmp -l 64 -m 2
./tecmp -l 128 -m 2
./tecmp -l 256 -m 2
./tecmp -l 512 -m 2

./rdcmp -n 8
./rdcmp -n 16
./rdcmp -n 32
./rdcmp -n 64
./rdcmp -n 128
./rdcmp -n 256
./rdcmp -n 512
./rdcmp -n 1024

./cdcmp -n 8
./cdcmp -n 16
./cdcmp -n 32
./cdcmp -n 64
./cdcmp -n 128
./cdcmp -n 256
./cdcmp -n 512
./cdcmp -n 1024

# pdte
cd ..
cd ..
bash build_pdte.sh
cd build


## pdte_with_adapted_sum_path

./tecmp_pdte_asm -t ../data/heart_11bits/model.json -v ../data/heart_11bits/x_test.csv -r 2048 -l 4 -m 3 -d 3 -e 1
./tecmp_pdte_asm -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 2048 -l 4 -m 3 -d 7
./tecmp_pdte_asm -t ../data/spam_11bits/model.json -v ../data/spam_11bits/x_test.csv -r 2048 -l 4 -m 3 -d 16
./tecmp_pdte_asm -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 2048 -l 4 -m 3 -d 10

./rdcmp_pdte_asm -t ../data/heart_11bits/model.json -v ../data/heart_11bits/x_test.csv -r 16383 -n 16 -d 3
./rdcmp_pdte_asm -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 16383 -n 16 -d 7
./rdcmp_pdte_asm -t ../data/spam_11bits/model.json -v ../data/spam_11bits/x_test.csv -r 16383 -n 16 -d 16
./rdcmp_pdte_asm -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 16383 -n 16 -d 10

./cdcmp_pdte_asm -t ../data/heart_11bits/model.json -v ../data/heart_11bits/x_test.csv -r 1024 -n 16 -d 3
./cdcmp_pdte_asm -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 1024 -n 16 -d 7
./cdcmp_pdte_asm -t ../data/spam_11bits/model.json -v ../data/spam_11bits/x_test.csv -r 1024 -n 16 -d 16
./cdcmp_pdte_asm -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 1024 -n 16 -d 10

## pdte_with_extended_sum_path

./tecmp_pdte_esm -t ../data/heart_11bits/model.json -v ../data/heart_11bits/x_test.csv -r 2048 -l 4 -m 3 -c 1 -e 10
./tecmp_pdte_esm -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 2048 -l 4 -m 3 -c 1 -e 10
./tecmp_pdte_esm -t ../data/spam_11bits/model.json -v ../data/spam_11bits/x_test.csv -r 2048 -l 4 -m 3 -c 1 -e 10
./tecmp_pdte_esm -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 2048 -l 4 -m 3 -c 1 -e 10

./rdcmp_pdte_esm -t ../data/heart_11bits/model.json -v ../data/heart_11bits/x_test.csv -r 512 -n 16 -c 1 -e 7
./rdcmp_pdte_esm -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 512 -n 16 -c 1 -e 7
./rdcmp_pdte_esm -t ../data/spam_11bits/model.json -v ../data/spam_11bits/x_test.csv -r 512 -n 16 -c 1 -e 7
./rdcmp_pdte_esm -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 512 -n 16 -c 1 -e 7

./cdcmp_pdte_esm -t ../data/heart_11bits/model.json -v ../data/heart_11bits/x_test.csv -r 256 -n 16 -c 1 -e 7
./cdcmp_pdte_esm -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 256 -n 16 -c 1 -e 7
./cdcmp_pdte_esm -t ../data/spam_11bits/model.json -v ../data/spam_11bits/x_test.csv -r 256 -n 16 -c 1 -e 7
./cdcmp_pdte_esm -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 256 -n 16 -c 1 -e 7

# compare with the Sortinghat

cd sortinghat_bench

cargo build --release

cargo run --release ../data/heart_11bits 10

cargo run --release ../data/breast_11bits 10

cargo run --release ../data/spam_11bits 10

cargo run --release ../data/electricity_10bits 10

# compare with the level up

cd level_up_bench

Should read level_up_bench/readme.md first.

# The other test, high-precision in single row

## new data 

mkdir ../data/heart_16bits 
./new_tree_and_data -i ../data/heart_11bits -o ../data/heart_16bits -n 16 -s 16384 

mkdir ../data/heart_32bits 
./new_tree_and_data -i ../data/heart_11bits -o ../data/heart_32bits -n 32 -s 16384 

mkdir ../data/breast_16bits 
./new_tree_and_data -i ../data/breast_11bits -o ../data/breast_16bits -n 16 -s 16384 

mkdir ../data/breast_32bits 
./new_tree_and_data -i ../data/breast_11bits -o ../data/breast_32bits -n 32 -s 16384 

## high-precision in single row

### tecmp-pdte

./tecmp_pdte_esm -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 1 -l 1 -m 12 -c 0
./tecmp_pdte_esm -t ../data/breast_16bits/model.json -v ../data/breast_16bits/x_test.csv -r 1 -l 2 -m 12 -c 0
./tecmp_pdte_esm -t ../data/breast_31bits/model.json -v ../data/breast_31bits/x_test.csv -r 1 -l 3 -m 12 -c 0


### cdcmp-pdte

./cdcmp_pdte_esm -t ../data/breast_16bits/model.json -v ../data/breast_11bits/x_test.csv -r 1 -n 16 -c 0
./cdcmp_pdte_esm -t ../data/breast_16bits/model.json -v ../data/breast_16bits/x_test.csv -r 1 -n 16 -c 0
./cdcmp_pdte_esm -t ../data/breast_31bits/model.json -v ../data/breast_31bits/x_test.csv -r 1 -n 32 -c 0
./cdcmp_pdte_esm -t ../data/breast_31bits/model.json -v ../data/breast_31bits/x_test.csv -r 1 -n 64 -c 0
./cdcmp_pdte_esm -t ../data/breast_31bits/model.json -v ../data/breast_31bits/x_test.csv -r 1 -n 128 -c 0

### depth need text

cd cmp_bench

g++ -o depth_test -O3 depth_test.cpp -I /usr/local/include/SEAL-4.1 -lseal-4.1

./depth_test

## the most bit precision

cd cmp_bench

./tecmp -l 2048 -m 13