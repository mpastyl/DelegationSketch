RUN_ON_ITHACA=0

buckets=512
rows=32

versions="cm_shared cm_local_copies cm_remote_inserts"

cd src
make clean
make all ITHACA=$RUN_ON_ITHACA
cd ../

thread_list="20"
query_rates="0"
collumns=`seq 10 10 200`

for version in $versions
do
    echo $version
    for queries in $query_rates
    do
        for threads in $thread_list
        do
            rm -f logs/size_${version}_${queries}_queries_${threads}_threads.log
            for col in $collumns
            do
                echo $version $queries $threads $col
                if [ "$version" != "cm_shared" ] && [ "$version" != "cm_shared_filtered" ]; then
                    ./bin/$version.out 10000 60000 $col $rows 3 1 0 1 $threads $queries 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >>  logs/size_${version}_${queries}_queries_${threads}_threads.log
                else
                    ./bin/$version.out 10000 60000 $(($col*$threads)) $rows 3 1 0 1 $threads $queries 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> logs/size_${version}_${queries}_queries_${threads}_threads.log
                fi
            done
        done
    done
done