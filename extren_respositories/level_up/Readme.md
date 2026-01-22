
# cmp

## cmp_bench

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

# pdte

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

# The other test, high-precision in single row

cd .. 
cd src
bash build_pdte.sh
cd build

## RCC, high precision 

./main  -t ../../../data/breast_11bits/model.json -v ../../../data/breast_11bits/x_test.csv -s ../../experiments/rcc-12-breast_11bits -n 12 -w 2 -e 0

./main  -t ../../../data/breast_16bits/model.json -v ../../../data/breast_16bits/x_test.csv -s ../../experiments/rcc-16-breast_16bits -n 16 -w 4 -e 0

./main  -t ../../../data/breast_31bits/model.json -v ../../../data/breast_31bits/x_test.csv -s ../../experiments/rcc-32-breast_31bits -n 32 -w 4 -e 0

./main  -t ../../../data/breast_31bits/model.json -v ../../../data/breast_31bits/x_test.csv -s ../../experiments/rcc-64-breast_31bits -n 64 -w 16 -e 0

./main  -t ../../../data/breast_31bits/model.json -v ../../../data/breast_31bits/x_test.csv -s ../../experiments/rcc-128-breast31bits -n 128 -w 16 -e 0

## FOLKLORE, high precision 

./main  -t ../../../data/breast_11bits/model.json -v ../../../data/breast_11bits/x_test.csv -s ../../experiments/folklore-12-breast_11bits -n 12 -w 0 -e 1

./main  -t ../../../data/breast_16bits/model.json -v ../../../data/breast_16bits/x_test.csv -s ../../experiments/folklore-16-breast_16bits -n 16 -w 0 -e 1

./main  -t ../../../data/breast_31bits/model.json -v ../../../data/breast_31bits/x_test.csv -s ../../experiments/folklore-32-breast_31bits -n 32 -w 0 -e 1

./main  -t ../../../data/breast_31bits/model.json -v ../../../data/breast_31bits/x_test.csv -s ../../experiments/folklore-64-breast_31bits -n 64 -w 0 -e 1

./main  -t ../../../data/breast_31bits/model.json -v ../../../data/breast_31bits/x_test.csv -s ../../experiments/folklore-128-breast_31bits -n 128 -w 0 -e 1

## XXCMP, high precision 

cd ..
cd ..
cd src2
bash build_pdte.sh
cd build

./main -v -m ../../../data/breast_11bits/model.json -a ../../../data/breast_11bits/x_test.csv -s ../../experiments/xxcmp-12-breast_11bits -n 12

./main -v -m ../../../data/breast_16bits/model.json -a ../../../data/breast_16bits/x_test.csv -s ../../experiments/xxcmp-16-breast_16bits -n 24

./main -v -m ../../../data/breast_31bits/model.json -a ../../../data/breast_31bits/x_test.csv -s ../../experiments/xxcmp-32-breast_31bits -n 32










