

# pdte

# cmp
## dcmp
## tcmp

# tree and data
## heart_11bits
## breast_11bits
## spam_11bits
## electricity_10bits

# dcmp
./pdte_main -i ../data/heart_11bits -l 1 -m 16 -d 10 -p 0
./pdte_main -i ../data/breast_11bits -l 1 -m 16 -d 10 -p 0
./pdte_main -i ../data/spam_11bits -l 1 -m 16 -d 10 -p 0
./pdte_main -i ../data/electricity_10bits -l 1 -m 16 -d 10 -p 0

./pdte_main -i ../data/heart_11bits -l 1 -m 16 -d 10 -p 2
./pdte_main -i ../data/breast_11bits -l 1 -m 16 -d 10 -p 2
./pdte_main -i ../data/spam_11bits -l 1 -m 16 -d 10 -p 2
./pdte_main -i ../data/electricity_10bits -l 1 -m 16 -d 10 -p 2

# tcmp
./pdte_main -i ../data/heart_11bits -l 1 -m 11 -d 10 -p 1
./pdte_main -i ../data/breast_11bits -l 1 -m 11 -d 10 -p 1
./pdte_main -i ../data/spam_11bits -l 1 -m 11 -d 10 -p 1
./pdte_main -i ../data/electricity_10bits -l 1 -m 10 -d 10 -p 1
