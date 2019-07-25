RUN_ON_ITHACA=0
thread_list=`seq 1 28`

buckets=512
rows=32

versions="cm_shared cm_local_copies cm_hybrid cm_remote_inserts cm_remote_inserts_filtered cm_shared_filtered cm_local_copies_filtered cm_augmented_sketch"

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
            if [ "$version" != "cm_shared" ] && [ "$version" != "cm_shared_filtered" ]; then
            echo "Using $rows * " $(($buckets/$threads))
                ./bin/$version.out 10000 60000 $(($buckets/$threads)) $rows 1 1 1 1 $threads $queries 0 >/dev/null
            else
                ./bin/$version.out 10000 60000 $buckets $rows 1 1 1 1 $threads $queries 0 >/dev/null
            fi
            cp logs/count_min_results.txt logs/${version}_${threads}_accuracy.log
        done
    done
done