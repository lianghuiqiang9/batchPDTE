

# bpdte
## bpdte-asm
## bpdte-esm

# tree and data
## heart_11bits
## breast_11bits
## spam_11bits
## electricity_10bits

# bpdte-asm
./build/bpdte_main -i ./data/heart_11bits -d 128 -l 2 -m 6
./build/bpdte_main -i ./data/breast_11bits -d 2048 -l 4 -m 3
./build/bpdte_main -i ./data/spam_11bits -d 2048 -l 4 -m 3
./build/bpdte_main -i ./data/electricity_10bits -d 2048 -l 4 -m 3

./build/bpdte_main -i ./data/heart_11bits -d 1024 -n 16 -c 1
./build/bpdte_main -i ./data/breast_11bits -d 1024 -n 16 -c 1
./build/bpdte_main -i ./data/spam_11bits -d 1024 -n 16 -c 1
./build/bpdte_main -i ./data/electricity_10bits -d 1024 -n 16 -c 1

./build/bpdte_main -i ./data/heart_11bits -d 16383 -n 16 -c 2 
./build/bpdte_main -i ./data/breast_11bits -d 16383 -n 16 -c 2 -e 1
./build/bpdte_main -i ./data/spam_11bits -d 16383 -n 16 -c 2
./build/bpdte_main -i ./data/electricity_10bits -d 16383 -n 16 -c 2 

# bpdte-esm
./build/bpdte_main -i ./data/heart_11bits -d 2048 -l 4 -m 3 -p 1 -e 9
./build/bpdte_main -i ./data/breast_11bits -d 2048 -l 4 -m 3 -p 1 -e 9
./build/bpdte_main -i ./data/spam_11bits -d 2048 -l 4 -m 3 -p 1 -e 9
# ./build/bpdte_main -i ./data/electricity_10bits -d 1024 -l 4 -m 3 -p 1 -e 9

./build/bpdte_main -i ./data/heart_11bits -d 512 -n 16 -c 1 -p 1 -e 6
./build/bpdte_main -i ./data/breast_11bits -d 256 -n 16 -c 1 -p 1 -e 6
./build/bpdte_main -i ./data/spam_11bits -d 256 -n 16 -c 1 -p 1 -e 6
# ./build/bpdte_main -i ./data/electricity_10bits -d 256 -n 16 -c 1 -p 1 -e 6

./build/bpdte_main -i ./data/heart_11bits -d 256 -n 16 -c 2 -p 1 -e 6
./build/bpdte_main -i ./data/breast_11bits -d 256 -n 16 -c 2 -p 1 -e 6
./build/bpdte_main -i ./data/spam_11bits -d 256 -n 16 -c 2 -p 1 -e 6
# ./build/bpdte_main -i ./data/electricity_10bits -d 128 -n 16 -c 2 -p 1 -e 6

# small batch size
./build/bpdte_main -i ./data/heart_11bits -d 2048 -l 2 -m 6

./build/bpdte_main -i ./data/heart_11bits -d 128 -l 2 -m 6
./build/bpdte_main -i ./data/breast_11bits -d 128 -l 2 -m 6
./build/bpdte_main -i ./data/spam_11bits -d 128 -l 2 -m 6
./build/bpdte_main -i ./data/electricity_10bits -d 128 -l 2 -m 6

./build/bpdte_main -i ./data/heart_11bits -d 8 -l 1 -m 10
./build/bpdte_main -i ./data/breast_11bits -d 8 -l 1 -m 10
./build/bpdte_main -i ./data/spam_11bits -d 8 -l 1 -m 10
./build/bpdte_main -i ./data/electricity_10bits -d 8 -l 1 -m 10

