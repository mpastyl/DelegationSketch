RUN_ON_ITHACA=0
thread_list=`seq 1 4 72`

buckets=256
rows=8

#ersions="cm_shared cm_local_copies cm_hybrid cm_remote_inserts cm_remote_inserts_filtered cm_shared_filtered cm_local_copies_filtered cm_augmented_sketch cm_delegation_filters cm_delegation_filters_with_linked_list"
versions="cm_shared cm_local_copies cm_local_copies cm_augmented_sketch cm_delegation_filters cm_delegation_filters_with_linked_list"

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
            if [ "$version" = "cm_shared" ] || [ "$version" = "cm_shared_filtered" ]; then
            	echo "Memory per thread" $(($buckets*$rows*4))
                ./bin/$version.out 10000 600000 $(($buckets*$threads)) $rows 1 1 0 1 $threads $queries 0 >/dev/null
            elif [ "$version" = "cm_delegation_filters" ] || [ "$version" = "cm_delegation_filters_with_linked_list" ]; then
	    	new_columns=$((($buckets*$rows*4 - $threads*64)/($rows*4)))
	    	echo "Memory per thread " $(($new_columns*$rows*4 + $threads*64))
	    	echo "Using " $new_columns "columns"
                ./bin/$version.out 10000 600000 $new_columns $rows 1 1 0 1 $threads $queries 0 >/dev/null
            else
		echo "Memory per thread" $(($buckets*$rows*4))
                ./bin/$version.out 10000 600000 $buckets $rows 1 1 0 1 $threads $queries 0 >/dev/null
            fi
            cp logs/count_min_results.txt logs/${version}_${threads}_accuracy.log
        done
    done
done
