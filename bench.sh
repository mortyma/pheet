make release
echo "n=1, g_2000_13000_3_10000_42"
./build/test/msp/bench/msp-bench -r 2 -n 1 --seq --strategy --strategy2 ../mcsp_doc/graphs/g_2000_13000_3_10000_42
echo "n=4, g_2000_13000_3_10000_42"
./build/test/msp/bench/msp-bench -r 2 -n 4 --seq --strategy --strategy2 ../mcsp_doc/graphs/g_2000_13000_3_10000_42
