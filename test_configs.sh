make clean
make all

versions="cm_shared cm_local_copies cm_hybrid cm_remote_inserts"


for version in $versions
do
    echo $version
    rm -f ${version}_no_queries.log
    for threads in `seq 1 28`
    do
        ./$version.out 10000 60000 64 64 1 1 1 1 $threads 0 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> ${version}_no_queries.log
    done
done

query_rates="0 2 4 6 8 10"
thread_list="20"
for version in $versions
do
    for threads  in $thread_list
    do
        echo ${version} ${threads}
        rm -f ${version}_${threads}_threads.log
        for queries in $query_rates
        do
            ./$version.out 10000 60000 64 64 1 1 1 1 $threads $queries 1 | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> ${version}_${threads}_threads.log
        done
    done
done