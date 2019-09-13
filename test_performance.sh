RUN_ON_ITHACA=1
thread_list=`seq 1 4 72`

buckets=512
rows=8

universe_size=1000000
stream_size=6000000


#versions="cm_shared cm_local_copies cm_hybrid cm_remote_inserts cm_remote_inserts_filtered cm_shared_filtered cm_local_copies_filtered cm_augmented_sketch cm_delegation_filters cm_delegation_filters_with_linked_list"
versions="cm_shared cm_local_copies cm_augmented_sketch cm_delegation_filters cm_delegation_filters_with_linked_list"

cd src
make clean
make all ITHACA=$RUN_ON_ITHACA
cd ../


query_rates="0 1 2 3"
for version in $versions
do
    echo $version
    for queries in $query_rates
    do
        rm -f logs/${version}_${queries}_queries.log
        for threads in $thread_list
        do
            if [ "$version" = "cm_shared" ] || [ "$version" = "cm_shared_filtered" ]; then
                ./bin/$version.out $universe_size $stream_size $(($buckets*$rows)) $rows 1 1.5 0 1 $threads $queries 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> logs/${version}_${queries}_queries.log
            else
                ./bin/$version.out $universe_size $stream_size $buckets $rows 1 1.5 0 1 $threads $queries 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> logs/${version}_${queries}_queries.log
            fi
        done
    done
done

query_rates="0 1 2 3 4 5 6"
thread_list="72"
for version in $versions
do
    for threads  in $thread_list
    do
        echo ${version} ${threads}
        rm -f logs/${version}_${threads}_threads.log
        for queries in $query_rates
        do
            if [ "$version" = "cm_shared" ] || [ "$version" = "cm_shared_filtered" ]; then
                ./bin/$version.out $universe_size $stream_size $(($buckets*$rows)) $rows 1 1.5 0 1 $threads $queries 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> logs/${version}_${threads}_threads.log
            else
                ./bin/$version.out $universe_size $stream_size $buckets $rows 1 1.5 0 1 $threads $queries 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> logs/${version}_${threads}_threads.log
            fi
        done
    done
done

skew_rates="0 0.25 0.5 0.75 1 1.25 1.5 1.75 2 2.25 2.5 2.75 3 3.25 3.5 3.75 4"
thread_list="72"
query_rates="0 1 2 3"
for version in $versions
do
    for threads  in $thread_list
    do
    for queries in $query_rates
    do
        echo ${version} ${threads}
        rm -f logs/skew_${version}_${threads}_threads_${queries}_queries.log
        for skew in $skew_rates
        do
            if [ "$version" = "cm_shared" ] || [ "$version" = "cm_shared_filtered" ]; then
                ./bin/$version.out $universe_size $stream_size $(($buckets*$rows)) $rows 1 $skew 0 1 $threads $queries 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> logs/skew_${version}_${threads}_threads_${queries}_queries.log
            else
                ./bin/$version.out $universe_size $stream_size $buckets $rows 1 $skew 0 1 $threads $queries 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> logs/skew_${version}_${threads}_threads_${queries}_queries.log
            fi
        done
    done
    done
done
