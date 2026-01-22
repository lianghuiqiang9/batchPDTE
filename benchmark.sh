
# build

g++ -o cmp_main -O3 cmp_main.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1 -L ./build -lpdte -Wl,-dpath,./lib

# tecmp
# cdcmp
# rdcmp

./cmp_main -c 0 -m 8 -l 2


./cmp_main -c 1 -n 8

./cmp_main -c 2 -n 8




g++ -o pdte_main -O3 pdte_main.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1 -L ./build -lpdte -Wl,-dpath,./lib


# heart_11bits
# breast_11bits
# spam_11bits
# electricity_10bits

# pdte

./pdte_main -i ./data/heart_11bits -d 2048 -l 8 -m 2 -n 16 -c 0 -e 0 

./pdte_main -i ./data/heart_11bits -d 512 -l 8 -m 2 -n 16 -c 1 -e 0

./pdte_main -i ./data/heart_11bits -d 8191 -l 8 -m 2 -n 16 -c 2 -e 0

./pdte_main -i ./data/breast_11bits -d 2048 -l 8 -m 2 -n 16 -c 0 -e 0

# pdte-esm

./pdte_main -i ./data/heart_11bits -d 1024 -l 4 -m 4 -n 16 -c 0 -e 7 -p 1

./pdte_main -i ./data/heart_11bits -d 256 -l 8 -m 2 -n 16 -c 1 -e 4 -p 1

./pdte_main -i ./data/heart_11bits -d 256 -l 8 -m 2 -n 16 -c 2 -e 4 -p 1

./pdte_main -i ./data/breast_11bits -d 1024 -l 8 -m 2 -n 16 -c 0 -e 5 -p 1


