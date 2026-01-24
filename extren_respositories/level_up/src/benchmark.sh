

./build/main  -t ../../../data/heart_11bits/model.json -v ../../../data/heart_11bits/x_test.csv -s ../experiments/rcc-heart_11bits -n 11 -w 4 -e 0

./build/main  -t ../../../data/breast_11bits/model.json -v ../../../data/breast_11bits/x_test.csv -s ../experiments/rcc-breast_11bits -n 11 -w 4 -e 0

./build/main  -t ../../../data/spam_11bits/model.json -v ../../../data/spam_11bits/x_test.csv -s ../experiments/rcc-spam_11bits -n 11 -w 4 -e 0

./build/main  -t ../../../data/electricity_10bits/model.json -v ../../../data/electricity_10bits/x_test.csv -s ../experiments/rcc-electricity_10bits -n 10 -w 4 -e 0

./build/main  -t ../../../data/heart_11bits/model.json -v ../../../data/heart_11bits/x_test.csv -s ../experiments/folklore-heart_11bits -n 11 -w 0 -e 1

./build/main  -t ../../../data/breast_11bits/model.json -v ../../../data/breast_11bits/x_test.csv -s ../experiments/folklore-breast_11bits -n 11 -w 0 -e 1

./build/main  -t ../../../data/spam_11bits/model.json -v ../../../data/spam_11bits/x_test.csv -s ../experiments/folklore-spam_11bits -n 11 -w 0 -e 1

./build/main -t ../../../data/electricity_10bits/model.json -v ../../../data/electricity_10bits/x_test.csv -s ../experiments/folklore-electricity_10bits -n 10 -w 0 -e 1

