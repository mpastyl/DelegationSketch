
CONFIG_UPDATE_ONLY_MINIMUM=0
CONFIG_ATOMIC_INCREMENTS=0
CONFIG_LOCAL_COPIES=1
CONFIG_FIXED_DURATION=1

cp params.h backup_params.h

sed -i -e "s/REPLACE_UPDATE_MIN/$CONFIG_UPDATE_ONLY_MINIMUM/g" params.h
sed -i -e "s/REPLACE_ATOMIC_INC/$CONFIG_ATOMIC_INCREMENTS/g" params.h
sed -i -e "s/REPLACE_LOCAL_COPIES/$CONFIG_LOCAL_COPIES/g" params.h
sed -i -e "s/REPLACE_FIXED_DURATION/$CONFIG_FIXED_DURATION/g" params.h

make clean
make cm_benchmark

#rm shared_no_queries.txt
#rm share_nothing_no_queries.txt

#for threads in `seq 1 32`
#do
#./cm_benchmark.out 10000 60000 64 64 1 1 0.5 1 $threads 0 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> shared_no_queries.txt
#./cm_benchmark.out 10000 60000 64 64 1 1 0.5 1 $threads 0 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> share_nothing_no_queries.txt
#done

query_rates="0 2 4 6 8 10"
for queries in $query_rates
do
#./cm_benchmark.out 10000 60000 64 64 1 1 0.5 1 10 $queries | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> shared_10_threads.txt
./cm_benchmark.out 10000 60000 64 64 1 1 0.5 1 10 $queries | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> share_nothing_10_threads.txt
done
mv backup_params.h params.h