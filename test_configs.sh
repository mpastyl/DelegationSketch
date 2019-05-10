make clean
make all

versions="cm_shared cm_local_copies cm_hybrid cm_remote_inserts"

buckets=512
rows=32

thread_list="20"
query_rates="0"
for version in $versions
do
    for threads  in $thread_list
    do
        echo ${version} ${threads}
        rm -f ${version}_${threads}_accuracy.log
        for queries in $query_rates
        do
            if [ "$version" != "cm_shared" ]; then
            echo "Using $rows * " $(($buckets/$threads))
                ./$version.out 10000 60000 $(($buckets/$threads)) $rows 1 1 1 1 $threads $queries 0 >/dev/null
            else
                ./$version.out 10000 60000 $buckets $rows 1 1 1 1 $threads $queries 0 >/dev/null
            fi
            cp count_min_results.txt ${version}_${threads}_accuracy.log
        done
    done
done

query_rates="0 2 4 6"
for version in $versions
do
    echo $version
    for queries in $query_rates
    do
        rm -f ${version}_${queries}_queries.log
        for threads in `seq 1 28`
        do
            if [ "$version" != "cm_shared" ]; then
                ./$version.out 10000 60000 $(($buckets/$threads)) $rows 1 1 0 1 $threads $queries 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> ${version}_${queries}_queries.log
            else
                ./$version.out 10000 60000 $buckets $rows 1 1 0 1 $threads $queries 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> ${version}_${queries}_queries.log
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
        rm -f ${version}_${threads}_threads.log
        for queries in $query_rates
        do
            if [ "$version" != "cm_shared" ]; then
                ./$version.out 10000 60000 $(($buckets/$threads)) $rows 1 1 0 1 $threads $queries 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> ${version}_${threads}_threads.log
            else
                ./$version.out 10000 60000 $buckets $rows 1 1 0 1 $threads $queries 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> ${version}_${threads}_threads.log
            fi
        done
    done
done