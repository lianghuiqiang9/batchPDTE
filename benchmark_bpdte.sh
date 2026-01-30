

# bpdte
## bpdte-asm
## bpdte-esm

# tree and data
## heart_11bits
## breast_11bits
## spam_11bits
## electricity_10bits

# bpdte-asm
./bpdte_main -i ../data/heart_11bits -d 128 -l 2 -m 6
./bpdte_main -i ../data/breast_11bits -d 2048 -l 4 -m 3
./bpdte_main -i ../data/spam_11bits -d 2048 -l 4 -m 3
./bpdte_main -i ../data/electricity_10bits -d 2048 -l 4 -m 3

./bpdte_main -i ../data/heart_11bits -d 1024 -l 1 -m 16 -c 1
./bpdte_main -i ../data/breast_11bits -d 1024 -l 1 -m 16 -c 1
./bpdte_main -i ../data/spam_11bits -d 1024 -l 1 -m 16 -c 1
./bpdte_main -i ../data/electricity_10bits -d 1024 -l 1 -m 16 -c 1

./bpdte_main -i ../data/heart_11bits -d 16383 -l 16 -m 1 -c 1
./bpdte_main -i ../data/breast_11bits -d 16383 -l 16 -m 1 -c 1 -e 1
./bpdte_main -i ../data/spam_11bits -d 16383 -l 16 -m 1 -c 1
./bpdte_main -i ../data/electricity_10bits -d 16383 -l 16 -m 1 -c 1

# bpdte-esm
./bpdte_main -i ../data/heart_11bits -d 2048 -l 4 -m 3 -p 1 -e 9
./bpdte_main -i ../data/breast_11bits -d 2048 -l 4 -m 3 -p 1 -e 9
./bpdte_main -i ../data/spam_11bits -d 2048 -l 4 -m 3 -p 1 -e 9
# ./bpdte_main -i ../data/electricity_10bits -d 1024 -l 4 -m 3 -p 1 -e 9

./bpdte_main -i ../data/heart_11bits -d 512 -l 1 -m 16 -c 1 -p 1 -e 6
./bpdte_main -i ../data/breast_11bits -d 256 -l 1 -m 16 -c 1 -p 1 -e 6
./bpdte_main -i ../data/spam_11bits -d 256 -l 1 -m 16 -c 1 -p 1 -e 6
# ./bpdte_main -i ../data/electricity_10bits -d 256 -l 1 -m 16 -c 1 -p 1 -e 6

./bpdte_main -i ../data/heart_11bits -d 256 -l 16 -m 1 -c 1 -p 1 -e 6
./bpdte_main -i ../data/breast_11bits -d 256 -l 16 -m 1 -c 1 -p 1 -e 6
./bpdte_main -i ../data/spam_11bits -d 256 -l 16 -m 1 -c 1 -p 1 -e 6
# ./bpdte_main -i ../data/electricity_10bits -d 128 -l 16 -m 1 -c 1 -p 1 -e 6

# small batch size
./bpdte_main -i ../data/heart_11bits -d 2048 -l 2 -m 6

./bpdte_main -i ../data/heart_11bits -d 128 -l 2 -m 6
./bpdte_main -i ../data/breast_11bits -d 128 -l 2 -m 6
./bpdte_main -i ../data/spam_11bits -d 128 -l 2 -m 6
./bpdte_main -i ../data/electricity_10bits -d 128 -l 2 -m 6

./bpdte_main -i ../data/heart_11bits -d 8 -l 1 -m 10
./bpdte_main -i ../data/breast_11bits -d 8 -l 1 -m 10
./bpdte_main -i ../data/spam_11bits -d 8 -l 1 -m 10
./bpdte_main -i ../data/electricity_10bits -d 8 -l 1 -m 10

