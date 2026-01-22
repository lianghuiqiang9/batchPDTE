
# build

mkdir build
cd build
make
make install

# run

./lhe_test -t 0

./lhe_depth_test -t 0

./cmp_main -m 8 -l 2 -n 16 -t 0

./serial_test -i ../data/heart_11bits -o ../data/heart_11bits_temp -d 16

./pdte_main -i ../data/heart_11bits -d 2048 -l 8 -m 2 -n 16 -c 0 -e 0 -p 0

./pdte_main -i ../data/heart_11bits -d 1024 -l 8 -m 2 -n 16 -c 0 -e 5 -p 1

# the params setting
1. -l 
2. -m
3. -n
4. -c
5. -

# benchmark


# the comparison

cd level_up_bench

cd ..

cd sortinghat_bench
