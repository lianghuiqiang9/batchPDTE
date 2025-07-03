
# Figure 5

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

# compare with the level up

cd level_up_bench

cd cmp_bench

g++ -o cmp -O3 cmp_bench.cpp utils.cpp -I /usr/local/include/SEAL-4.1 -lseal-4.1

./cmp -n 8
./cmp -n 12
./cmp -n 16
./cmp -n 32
./cmp -n 64
./cmp -n 128
./cmp -n 256
./cmp -n 512
./cmp -n 1024


# Figure 6


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

# Table 8 

# compare with the Sortinghat

cd sortinghat_bench

cargo build --release

cargo run --release ../data/heart_11bits 10

cargo run --release ../data/breast_11bits 10

cargo run --release ../data/spam_11bits 10

cargo run --release ../data/electricity_10bits 10

# compare with the level up

cd level_up_bench

cd .. 
cd src
bash build_pdte.sh
cd build

## RCC

./main  -t ../../../data/heart_11bits/model.json -v ../../../data/heart_11bits/x_test.csv -s ../../experiments/rcc-heart_11bits -n 11 -w 4 -e 0

./main  -t ../../../data/breast_11bits/model.json -v ../../../data/breast_11bits/x_test.csv -s ../../experiments/rcc-breast_11bits -n 11 -w 4 -e 0

./main  -t ../../../data/spam_11bits/model.json -v ../../../data/spam_11bits/x_test.csv -s ../../experiments/rcc-spam_11bits -n 11 -w 4 -e 0

./main  -t ../../data/electricity_10bits/model.json -v ../../data/electricity_10bits/x_test.csv -s ../../experiments/rcc-electricity_10bits -n 10 -w 4 -e 0

## FOLKLORE

./main  -t ../../../data/heart_11bits/model.json -v ../../../data/heart_11bits/x_test.csv -s ../../experiments/folklore-heart_11bits -n 11 -w 0 -e 1

./main  -t ../../../data/breast_11bits/model.json -v ../../../data/breast_11bits/x_test.csv -s ../../experiments/folklore-breast_11bits -n 11 -w 0 -e 1

./main  -t ../../../data/spam_11bits/model.json -v ../../../data/spam_11bits/x_test.csv -s ../../experiments/folklore-spam_11bits -n 11 -w 0 -e 1

./main  -t ../../../data/electricity_10bits/model.json -v ../../../data/electricity_10bits/x_test.csv -s ../../experiments/folklore-electricity_10bits -n 10 -w 0 -e 1

## XXCMP

cd ..
cd ..
cd src2
bash build_pdte.sh
cd build

./main -v -m ../../../data/heart_11bits/model.json -a ../../../data/heart_11bits/x_test.csv -s ../../experiments/xxcmp-heart_11bits -n 11

./main -v -m ../../../data/breast_11bits/model.json -a ../../../data/breast_11bits/x_test.csv -s ../../experiments/xxcmp-breast_11bits -n 11

./main -v -m ../../../data/spam_11bits/model.json -a ../../../data/spam_11bits/x_test.csv -s ../../experiments/xxcmp-spam_11bits -n 11

./main -v -m ../../../data/electricity_10bits/model.json -a ../../../data/electricity_10bits/x_test.csv -s ../../experiments/xxcmp-electricity_10bits -n 10

# Table 9

# ours

./tecmp_pdte_asm -t ../data/heart_11bits/model.json -v ../data/heart_11bits/x_test.csv -r 4 -l 1 -m 11 -d 3 
./tecmp_pdte_asm -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 4 -l 1 -m 11 -d 7
./tecmp_pdte_asm -t ../data/spam_11bits/model.json -v ../data/spam_11bits/x_test.csv -r 4 -l 1 -m 11 -d 16
./tecmp_pdte_asm -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 4 -l 1 -m 10 -d 10 -e 1

./tecmp_pdte_asm -t ../data/heart_11bits/model.json -v ../data/heart_11bits/x_test.csv -r 256 -l 2 -m 6 -d 3 -e 2
./tecmp_pdte_asm -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 256 -l 2 -m 6 -d 7 -e 1
./tecmp_pdte_asm -t ../data/spam_11bits/model.json -v ../data/spam_11bits/x_test.csv -r 256 -l 2 -m 6 -d 16
./tecmp_pdte_asm -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 256 -l 2 -m 5 -d 10 -e 1

./tecmp_pdte_asm -t ../data/heart_11bits/model.json -v ../data/heart_11bits/x_test.csv -r 2048 -l 4 -m 3 -d 3 -e 1
./tecmp_pdte_asm -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 2048 -l 4 -m 3 -d 7
./tecmp_pdte_asm -t ../data/spam_11bits/model.json -v ../data/spam_11bits/x_test.csv -r 2048 -l 4 -m 3 -d 16
./tecmp_pdte_asm -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 2048 -l 4 -m 3 -d 10

