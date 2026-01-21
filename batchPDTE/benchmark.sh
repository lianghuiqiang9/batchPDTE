
# build

g++ -o cmp_main -O3 cmp_main.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1 -L ./build -lpdte -Wl,-rpath,./lib


g++ -o pdte_main -O3 pdte_main.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1 -L ./build -lpdte -Wl,-rpath,./lib


# heart_11bits
# breast_11bits
# spam_11bits
# electricity_10bits

# pdte

./pdte_main -i ./data/heart_11bits -r 2048 -t 0 -l 8 -m 2 -n 16 -e 0 

./pdte_main -i ./data/heart_11bits -r 512 -t 1 -l 8 -m 2 -n 16 -e 0

./pdte_main -i ./data/heart_11bits -r 8191 -t 2 -l 8 -m 2 -n 16 -e 0

./pdte_main -i ./data/breast_11bits -r 2048 -t 0 -l 8 -m 2 -n 16 -e 0

# pdte-esm

./pdte_main -i ./data/heart_11bits -r 1024 -t 0 -l 4 -m 4 -n 16 -e 7 -s 1

./pdte_main -i ./data/heart_11bits -r 256 -t 1 -l 8 -m 2 -n 16 -e 4 -s 1

./pdte_main -i ./data/heart_11bits -r 256 -t 2 -l 8 -m 2 -n 16 -e 4 -s 1

./pdte_main -i ./data/breast_11bits -r 1024 -t 0 -l 8 -m 2 -n 16 -e 5 -s 1


