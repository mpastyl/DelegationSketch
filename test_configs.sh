RUN_ON_ITHACA=0
thread_list=`seq 1 28`

buckets=512
rows=32

versions="cm_shared cm_local_copies cm_hybrid cm_remote_inserts cm_remote_inserts_filtered cm_shared_filtered"

cd src
make clean
make all ITHACA=$RUN_ON_ITHACA
cd ../

query_rates="0"
for version in $versions
do
    for threads  in $thread_list
    do
        echo ${version} ${threads}
        rm -f logs/${version}_${threads}_accuracy.log
        for queries in $query_rates
        do
            if [ "$version" != "cm_shared" ]; then
            echo "Using $rows * " $(($buckets/$threads))
                ./bin/$version.out 10000 60000 $(($buckets/$threads)) $rows 1 1 1 1 $threads $queries 0 >/dev/null
            else
                ./bin/$version.out 10000 60000 $buckets $rows 1 1 1 1 $threads $queries 0 >/dev/null
            fi
            cp logs/count_min_results.txt logs/${version}_${threads}_accuracy.log
        done
    done
done

query_rates="0 2 4 6"
for version in $versions
do
    echo $version
    for queries in $query_rates
    do
        rm -f logs/${version}_${queries}_queries.log
        for threads in $thread_list
        do
            if [ "$version" != "cm_shared" ]; then
                ./bin/$version.out 10000 60000 $(($buckets/$threads)) $rows 1 1 0 1 $threads $queries 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> logs/${version}_${queries}_queries.log
            else
                ./bin/$version.out 10000 60000 $buckets $rows 1 1 0 1 $threads $queries 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> logs/${version}_${queries}_queries.log
            fi
        done
    done
done

query_rates="0 2 4 6"
thread_list="20"
for version in $versions
do
    for threads  in $thread_list
    do
        echo ${version} ${threads}
        rm -f logs/${version}_${threads}_threads.log
        for queries in $query_rates
        do
            if [ "$version" != "cm_shared" ]; then
                ./bin/$version.out 10000 60000 $(($buckets/$threads)) $rows 1 1 0 1 $threads $queries 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> logs/${version}_${threads}_threads.log
            else
                ./bin/$version.out 10000 60000 $buckets $rows 1 1 0 1 $threads $queries 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> logs/${version}_${threads}_threads.log
            fi
        done
    done
done