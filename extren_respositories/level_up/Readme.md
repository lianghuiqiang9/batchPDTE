
# cmp

## cmp_bench

cd cmp_bench

g++ -o cmp -O3 cmp_bench.cpp utils.cpp -I /usr/local/include/SEAL-4.1 -lseal-4.1

./cmp -n 8
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

## RCC

./build/main  -t ../../../data/heart_11bits/model.json -v ../../../data/heart_11bits/x_test.csv -s ../experiments/rcc-heart_11bits -n 11 -w 4 -e 0

./build/main  -t ../../../data/breast_11bits/model.json -v ../../../data/breast_11bits/x_test.csv -s ../experiments/rcc-breast_11bits -n 11 -w 4 -e 0

./build/main  -t ../../../data/spam_11bits/model.json -v ../../../data/spam_11bits/x_test.csv -s ../experiments/rcc-spam_11bits -n 11 -w 4 -e 0

./build/main  -t ../../../data/electricity_10bits/model.json -v ../../../data/electricity_10bits/x_test.csv -s ../experiments/rcc-electricity_10bits -n 10 -w 4 -e 0

## FOLKLORE

./build/main  -t ../../../data/heart_11bits/model.json -v ../../../data/heart_11bits/x_test.csv -s ../experiments/folklore-heart_11bits -n 11 -w 0 -e 1

./build/main  -t ../../../data/breast_11bits/model.json -v ../../../data/breast_11bits/x_test.csv -s ../experiments/folklore-breast_11bits -n 11 -w 0 -e 1

./build/main  -t ../../../data/spam_11bits/model.json -v ../../../data/spam_11bits/x_test.csv -s ../experiments/folklore-spam_11bits -n 11 -w 0 -e 1

./build/main -t ../../../data/electricity_10bits/model.json -v ../../../data/electricity_10bits/x_test.csv -s ../experiments/folklore-electricity_10bits -n 10 -w 0 -e 1

## XXCMP


cd ..
cd src2
bash build_pdte.sh
cd build

./build/main -v -m ../../../data/heart_11bits/model.json -a ../../../data/heart_11bits/x_test.csv -s ../experiments/xxcmp-heart_11bits -n 11

./build/main -v -m ../../../data/breast_11bits/model.json -a ../../../data/breast_11bits/x_test.csv -s ../experiments/xxcmp-breast_11bits -n 11

./build/main -v -m ../../../data/spam_11bits/model.json -a ../../../data/spam_11bits/x_test.csv -s ../experiments/xxcmp-spam_11bits -n 11

./build/main -v -m ../../../data/electricity_10bits/model.json -a ../../../data/electricity_10bits/x_test.csv -s ../experiments/xxcmp-electricity_10bits -n 10






