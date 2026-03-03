

# pdte

# cmp
## dcmp
## tcmp

# tree and data
## heart_11bits
## breast_11bits
## spam_11bits
## electricity_10bits

# dcmp-sumpath
# root-to-neaf path
# ./pdte_main -i ../data/heart_11bits -n 11 -m 11 -d 10 -p 0
# ./pdte_main -i ../data/breast_11bits -n 11 -m 11 -d 10 -p 0
# ./pdte_main -i ../data/spam_11bits -n 11 -m 11 -d 10 -p 0
# ./pdte_main -i ../data/electricity_10bits -n 10 -m 10 -d 10 -p 0

# tcmp-sumpath2
# table 7
# node-by-node
./pdte_main -i ../data/heart_11bits -n 11 -m 11 -d 10 -p 1
./pdte_main -i ../data/breast_11bits -n 11 -m 11 -d 10 -p 1
./pdte_main -i ../data/spam_11bits -n 11 -m 11 -d 10 -p 1
./pdte_main -i ../data/electricity_10bits -n 10 -m 10 -d 10 -p 1

# dcmp-sumpath2
# node-by-node
# ./pdte_main -i ../data/heart_11bits -n 11 -m 11 -d 10 -p 1 -c 1
# ./pdte_main -i ../data/breast_11bits -n 11 -m 11 -d 10 -p 1 c 1
# ./pdte_main -i ../data/spam_11bits -n 11 -m 11 -d 10 -p 1 -c 1
# ./pdte_main -i ../data/electricity_10bits -n 11 -m 10 -d 10 -p 1 -c 1

# dcmp-multipath
# node-by-node
# ./pdte_main -i ../data/heart_11bits -n 11 -m 11 -d 10 -p 2 -c 1
# ./pdte_main -i ../data/breast_11bits -n 11 -m 11 -d 10 -p 2 -c 1
# ./pdte_main -i ../data/spam_11bits -n 11 -m 11 -d 10 -p 2 -c 1
# ./pdte_main -i ../data/electricity_10bits -n 10 -m 10 -d 10 -p 2 -c 1