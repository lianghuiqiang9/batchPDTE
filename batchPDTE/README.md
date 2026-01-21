
# build

mkdir build
cd build
make
make install

# run

./lhe_test

./lhe_depth_test

./cmp_main -m 8 -l 2 -n 16 -t 0

./serial_test -i ../data/heart_11bits -o ../data/heart_11bits_temp -r 16

./pdte_main -i ../data/heart_11bits -r 2048 -t 0 -l 8 -m 2 -n 16 -e 0

# benchmark


# the comparison

cd level_up_bench

cd ..

cd sortinghat_bench
