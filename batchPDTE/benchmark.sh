
# build

g++ -o pdte_test -O3 pdte_test.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1


# heart_11bits
# breast_11bits
# spam_11bits
# electricity_10bits

# pdte

./pdte_test -i ./data/heart_11bits -s 2048 -t 0 -l 8 -m 2 -n 16 -e 0

./pdte_test -i ./data/heart_11bits -s 512 -t 1 -l 8 -m 2 -n 16 -e 0

./pdte_test -i ./data/heart_11bits -s 8191 -t 2 -l 8 -m 2 -n 16 -e 0

./pdte_test -i ./data/breast_11bits -s 2048 -t 0 -l 8 -m 2 -n 16 -e 0

