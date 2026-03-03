# bpdte
## bpdte-asm
## bpdte-esm

# tree and data
## heart_11bits
## breast_11bits
## spam_11bits
## electricity_10bits

# bpdte-asm
# table 8
# bpdte_cw (h = 2)
./bpdte_main -i ../data/heart_11bits -d 16384 -n 11 -m 1 -h 2 -c 2
./bpdte_main -i ../data/breast_11bits -d 16384 -n 11 -m 1 -h 2 -c 2
./bpdte_main -i ../data/spam_11bits -d 16384 -n 11 -m 1 -h 2 -c 2
./bpdte_main -i ../data/electricity_10bits -d 16384 -n 10 -m 1 -h 2 -c 2

# bpdte_cw (h = 4)-bpdte
./bpdte_main -i ../data/heart_11bits -d 16384 -n 11 -m 1 -h 4 -c 2
./bpdte_main -i ../data/breast_11bits -d 16384 -n 11 -m 1 -h 4 -c 2
./bpdte_main -i ../data/spam_11bits -d 16384 -n 11 -m 1 -h 4 -c 2
./bpdte_main -i ../data/electricity_10bits -d 16384 -n 10 -m 1 -h 4 -c 2

# tcmp (m = 2)-bpdte
./bpdte_main -i ../data/heart_11bits -d 4096 -n 11 -m 2
./bpdte_main -i ../data/breast_11bits -d 4096 -n 11 -m 2
./bpdte_main -i ../data/spam_11bits -d 4096 -n 11 -m 2
./bpdte_main -i ../data/electricity_10bits -d 4096 -n 10 -m 2

# tcmp (m = 4)-bpdte
./bpdte_main -i ../data/heart_11bits -d 1024 -n 11 -m 4
./bpdte_main -i ../data/breast_11bits -d 1024 -n 11 -m 4
./bpdte_main -i ../data/spam_11bits -d 1024 -n 11 -m 4
./bpdte_main -i ../data/electricity_10bits -d 1024 -n 10 -m 4

# dcmp (m = 1)-bpdte
./bpdte_main -i ../data/heart_11bits -d 16384 -n 11 -m 1 -c 1
./bpdte_main -i ../data/breast_11bits -d 16384 -n 11 -m 1 -c 1 -e 1
./bpdte_main -i ../data/spam_11bits -d 16384 -n 11 -m 1 -c 1
./bpdte_main -i ../data/electricity_10bits -d 16384 -n 10 -m 1 -c 1

# dcmp (m = n)-bpdte
./bpdte_main -i ../data/heart_11bits -d 1024 -n 11 -m 11 -c 1
./bpdte_main -i ../data/breast_11bits -d 1024 -n 11 -m 11 -c 1
./bpdte_main -i ../data/spam_11bits -d 1024 -n 11 -m 11 -c 1
./bpdte_main -i ../data/electricity_10bits -d 1024 -n 10 -m 10 -c 1

# ./bpdte_main -i ../data/heart_11bits -d 128 -n 2 -m 6
# ./bpdte_main -i ../data/electricity_10bits -d 128 -n 2 -m 6
# ./bpdte_main -i ../data/electricity_10bits -d 2048 -n 4 -m 3
# ./bpdte_main -i ../data/electricity_10bits -d 1024 -n 1 -m 16 -c 1

# different batch size
# Figure 7
# t-pdte
./pdte_main -i ../data/electricity_10bits -n 10 -m 10 -d 1 -p 1
./pdte_main -i ../data/electricity_10bits -n 10 -m 10 -d 2 -p 1
./pdte_main -i ../data/electricity_10bits -n 10 -m 10 -d 4 -p 1
./pdte_main -i ../data/electricity_10bits -n 10 -m 10 -d 8 -p 1
./pdte_main -i ../data/electricity_10bits -n 10 -m 10 -d 16 -p 1

# t-bpdte
./bpdte_main -i ../data/electricity_10bits -d 512 -n 10 -m 5
./bpdte_main -i ../data/electricity_10bits -d 1024 -n 10 -m 4
./bpdte_main -i ../data/electricity_10bits -d 2048 -n 10 -m 3
./bpdte_main -i ../data/electricity_10bits -d 4096 -n 10 -m 2

# d-bpdte
./bpdte_main -i ../data/electricity_10bits -d 1024 -n 10 -m 10 -c 1
./bpdte_main -i ../data/electricity_10bits -d 2048 -n 10 -m 8 -c 1
./bpdte_main -i ../data/electricity_10bits -d 4096 -n 10 -m 4 -c 1
./bpdte_main -i ../data/electricity_10bits -d 8192 -n 10 -m 2 -c 1
./bpdte_main -i ../data/electricity_10bits -d 16384 -n 10 -m 1 -c 1

# bpdte_cw (h = 2)
./bpdte_main -i ../data/electricity_10bits -d 16384 -n 10 -m 1 -h 2 -c 2

# bpdte_cw (h = 4)
./bpdte_main -i ../data/electricity_10bits -d 16384 -n 10 -m 1 -h 4 -c 2

# bpdte-esm
# ./bpdte_main -i ../data/heart_11bits -d 2048 -n 11 -m 3 -p 1 -e 9
# ./bpdte_main -i ../data/breast_11bits -d 2048 -n 11 -m 3 -p 1 -e 9
# ./bpdte_main -i ../data/spam_11bits -d 2048 -n 11 -m 3 -p 1 -e 9
# ./bpdte_main -i ../data/electricity_10bits -d 1024 -n 10 -m 3 -p 1 -e 9

# ./bpdte_main -i ../data/heart_11bits -d 512 -n 11 -m 11 -c 1 -p 1 -e 6
# ./bpdte_main -i ../data/breast_11bits -d 256 -n 11 -m 11 -c 1 -p 1 -e 6
# ./bpdte_main -i ../data/spam_11bits -d 256 -n 11 -m 11 -c 1 -p 1 -e 6
# ./bpdte_main -i ../data/electricity_10bits -d 256 -n 10 -m 10 -c 1 -p 1 -e 6

# small batch size
# ./bpdte_main -i ../data/heart_11bits -d 2048 -n 11 -m 6

# ./bpdte_main -i ../data/heart_11bits -d 128 -n 11 -m 6
# ./bpdte_main -i ../data/breast_11bits -d 128 -n 11 -m 6
# ./bpdte_main -i ../data/spam_11bits -d 128 -n 11 -m 6
# ./bpdte_main -i ../data/electricity_10bits -d 128 -n 11 -m 6

# ./bpdte_main -i ../data/heart_11bits -d 8 -n 11 -m 10
# ./bpdte_main -i ../data/breast_11bits -d 8 -n 11 -m 10
# ./bpdte_main -i ../data/spam_11bits -d 8 -n 11 -m 10
# ./bpdte_main -i ../data/electricity_10bits -d 8 -n 11 -m 10

