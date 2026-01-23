
# tecmp
# cdcmp
# rdcmp

./cmp_main -c 0 -m 4 -l 2
./cmp_main -c 0 -m 4 -l 4
./cmp_main -c 0 -m 4 -l 8
./cmp_main -c 0 -m 4 -l 16
./cmp_main -c 0 -m 4 -l 32
./cmp_main -c 0 -m 4 -l 64
./cmp_main -c 0 -m 4 -l 128
./cmp_main -c 0 -m 4 -l 256

./cmp_main -c 1 -n 8
./cmp_main -c 1 -n 16
./cmp_main -c 1 -n 32
./cmp_main -c 1 -n 64
./cmp_main -c 1 -n 128
./cmp_main -c 1 -n 256
./cmp_main -c 1 -n 512
./cmp_main -c 1 -n 1024

./cmp_main -c 2 -n 8
./cmp_main -c 2 -n 16
./cmp_main -c 2 -n 32
./cmp_main -c 2 -n 64
./cmp_main -c 2 -n 128
./cmp_main -c 2 -n 256
./cmp_main -c 2 -n 512
./cmp_main -c 2 -n 1024

# heart_11bits
# breast_11bits
# spam_11bits
# electricity_10bits

# pdte-asm
./pdte_main -i ../data/heart_11bits -d 128 -l 2 -m 6
./pdte_main -i ../data/breast_11bits -d 2048 -l 4 -m 3
./pdte_main -i ../data/spam_11bits -d 2048 -l 4 -m 3
./pdte_main -i ../data/electricity_10bits -d 2048 -l 4 -m 3

./pdte_main -i ../data/heart_11bits -d 1024 -n 16 -c 1
./pdte_main -i ../data/breast_11bits -d 1024 -n 16 -c 1
./pdte_main -i ../data/spam_11bits -d 1024 -n 16 -c 1
./pdte_main -i ../data/electricity_10bits -d 1024 -n 16 -c 1

./pdte_main -i ../data/heart_11bits -d 16383 -n 16 -c 2 
./pdte_main -i ../data/breast_11bits -d 16383 -n 16 -c 2 -e 1
./pdte_main -i ../data/spam_11bits -d 16383 -n 16 -c 2
./pdte_main -i ../data/electricity_10bits -d 16383 -n 16 -c 2 

# pdte-esm
./pdte_main -i ../data/heart_11bits -d 2048 -l 4 -m 3 -p 1 -e 9
./pdte_main -i ../data/breast_11bits -d 2048 -l 4 -m 3 -p 1 -e 9
./pdte_main -i ../data/spam_11bits -d 2048 -l 4 -m 3 -p 1 -e 9
# ./pdte_main -i ../data/electricity_10bits -d 1024 -l 4 -m 3 -p 1 -e 9

./pdte_main -i ../data/heart_11bits -d 512 -n 16 -c 1 -p 1 -e 6
./pdte_main -i ../data/breast_11bits -d 256 -n 16 -c 1 -p 1 -e 6
./pdte_main -i ../data/spam_11bits -d 256 -n 16 -c 1 -p 1 -e 6
# ./pdte_main -i ../data/electricity_10bits -d 256 -n 16 -c 1 -p 1 -e 6

./pdte_main -i ../data/heart_11bits -d 256 -n 16 -c 2 -p 1 -e 6
./pdte_main -i ../data/breast_11bits -d 256 -n 16 -c 2 -p 1 -e 6
./pdte_main -i ../data/spam_11bits -d 256 -n 16 -c 2 -p 1 -e 6
# ./pdte_main -i ../data/electricity_10bits -d 128 -n 16 -c 2 -p 1 -e 6

