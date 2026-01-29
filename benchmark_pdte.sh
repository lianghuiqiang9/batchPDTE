

# pdte

# dcmp

# tree and data
## heart_11bits
## breast_11bits
## spam_11bits
## electricity_10bits

./build/pdte_main -i ./data/heart_11bits -l 1 -m 16 -d 10
./build/pdte_main -i ./data/breast_11bits -l 1 -m 16 -d 10
./build/pdte_main -i ./data/spam_11bits -l 1 -m 16 -d 10
./build/pdte_main -i ./data/electricity_10bits -l 1 -m 16 -d 10

./build/pdte_main -i ./data/heart_11bits -l 1 -m 16 -d 10 -p 1
./build/pdte_main -i ./data/breast_11bits -l 1 -m 16 -d 10 -p 1
./build/pdte_main -i ./data/spam_11bits -l 1 -m 16 -d 10 -p 1
./build/pdte_main -i ./data/electricity_10bits -l 1 -m 16 -d 10 -p 1