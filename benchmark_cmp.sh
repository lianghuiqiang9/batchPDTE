
# cmp
## tcmp
## dcmp
## cwcmp

# precision 
# figure 5
# tcmp m = 2
./cmp_main -c 0 -n 8 -m 2 
./cmp_main -c 0 -n 16 -m 2
./cmp_main -c 0 -n 32 -m 2
./cmp_main -c 0 -n 64 -m 2
./cmp_main -c 0 -n 128 -m 2
./cmp_main -c 0 -n 256 -m 2
./cmp_main -c 0 -n 512 -m 2
./cmp_main -c 0 -n 1024 -m 2

# dcmp m = n
./cmp_main -c 1 -n 8 -m 8
./cmp_main -c 1 -n 16 -m 16
./cmp_main -c 1 -n 32 -m 32
./cmp_main -c 1 -n 64 -m 64
./cmp_main -c 1 -n 128 -m 128
./cmp_main -c 1 -n 256 -m 256
./cmp_main -c 1 -n 512 -m 512
./cmp_main -c 1 -n 1024 -m 1024

# dcmp m = 1
./cmp_main -c 1 -n 8 -m 1
./cmp_main -c 1 -n 16 -m 1
./cmp_main -c 1 -n 32 -m 1
./cmp_main -c 1 -n 64 -m 1
./cmp_main -c 1 -n 128 -m 1
./cmp_main -c 1 -n 256 -m 1
./cmp_main -c 1 -n 512 -m 1
./cmp_main -c 1 -n 1024 -m 1

# cwcmp
./cmp_main -c 2 -n 8 -h 8
./cmp_main -c 2 -n 16 -h 8
./cmp_main -c 2 -n 32 -h 8 -e 2

# 32bits
# figure 6
# tcmp
./cmp_main -c 0 -n 32 -m 2
./cmp_main -c 0 -n 32 -m 4
./cmp_main -c 0 -n 32 -m 6
./cmp_main -c 0 -n 32 -m 8
# dcmp
./cmp_main -c 1 -n 32 -m 32
./cmp_main -c 1 -n 32 -m 16
./cmp_main -c 1 -n 32 -m 8
./cmp_main -c 1 -n 32 -m 4
./cmp_main -c 1 -n 32 -m 2
./cmp_main -c 1 -n 32 -m 1
# cwcmp
./cmp_main -c 2 -n 32 -h 4
./cmp_main -c 2 -n 32 -h 6
./cmp_main -c 2 -n 32 -h 8 -e 2


# large precision
# tcmp
./cmp_main -c 0 -n 12 -m 12
./cmp_main -c 0 -n 24 -m 12
./cmp_main -c 0 -n 52 -m 13
./cmp_main -c 0 -n 104 -m 13
./cmp_main -c 0 -n 208 -m 13
./cmp_main -c 0 -n 448 -m 14
./cmp_main -c 0 -n 896 -m 14