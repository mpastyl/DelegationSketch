RUN_ON_ITHACA=1
thread_list="72"

buckets=512 #use 800 for odysseus, 512 for ithaca
rows=8

universe_size=1000000
stream_size=6000000


#versions="cm_shared cm_local_copies cm_hybrid cm_remote_inserts cm_remote_inserts_filtered cm_shared_filtered cm_local_copies_filtered cm_augmented_sketch cm_delegation_filters cm_delegation_filters_with_linked_list"
versions="cm_shared cm_local_copies cm_augmented_sketch cm_delegation_filters cm_delegation_filters_with_linked_list"
#versions="cm_delegation_filters_with_linked_list"

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
        rm -f logs/${version}_${queries}_queries_real_data_CAIDA_2018_source_ports_10_times_final.log
        for threads in $thread_list
        do
	for rep in `seq 1 10`
	do
            if [ "$version" = "cm_shared" ] || [ "$version" = "cm_shared_filtered" ]; then
                ./bin/$version.out $universe_size $stream_size $(($buckets*$rows)) $rows 1 1.5 0 1 $threads $queries 1 /home/chasty/CAIDA_2018_source_ports | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> logs/${version}_${queries}_queries_real_data_CAIDA_2018_source_ports_10_times_final.log
            elif [ "$version" = "cm_delegation_filters" ] || [ "$version" = "cm_delegation_filters_with_linked_list" ]; then
                new_columns=$((($buckets*$rows*4 - $threads*64)/($rows*4))) 
                ./bin/$version.out $universe_size $stream_size $new_columns $rows 1 1.5 0 1 $threads $queries 1 /home/chasty/CAIDA_2018_source_ports | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> logs/${version}_${queries}_queries_real_data_CAIDA_2018_source_ports_10_times_final.log
            else
                ./bin/$version.out $universe_size $stream_size $buckets $rows 1 1.5 0 1 $threads $queries 1 /home/chasty/CAIDA_2018_source_ports | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> logs/${version}_${queries}_queries_real_data_CAIDA_2018_source_ports_10_times_final.log
            fi
        done
	done
    done
done

query_rates="0 1 2 3"
for version in $versions
do
    echo $version
    for queries in $query_rates
    do
        rm -f logs/${version}_${queries}_queries_real_data_CAIDA_2018_source_ips_10_times_final.log
        for threads in $thread_list
        do
	for rep in `seq 1 10`
	do
            if [ "$version" = "cm_shared" ] || [ "$version" = "cm_shared_filtered" ]; then
                ./bin/$version.out $universe_size $stream_size $(($buckets*$rows)) $rows 1 1.5 0 1 $threads $queries 1 /home/chasty/CAIDA_2018_source_ips | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> logs/${version}_${queries}_queries_real_data_CAIDA_2018_source_ips_10_times_final.log
            elif [ "$version" = "cm_delegation_filters" ] || [ "$version" = "cm_delegation_filters_with_linked_list" ]; then
                new_columns=$((($buckets*$rows*4 - $threads*64)/($rows*4))) 
                ./bin/$version.out $universe_size $stream_size $new_columns $rows 1 1.5 0 1 $threads $queries 1 /home/chasty/CAIDA_2018_source_ips | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> logs/${version}_${queries}_queries_real_data_CAIDA_2018_source_ips_10_times_final.log
            else
                ./bin/$version.out $universe_size $stream_size $buckets $rows 1 1.5 0 1 $threads $queries 1 /home/chasty/CAIDA_2018_source_ips | grep -oP 'Total processing throughput [+-]?[0-9]+([.][0-9]+)?+' -a --text >> logs/${version}_${queries}_queries_real_data_CAIDA_2018_source_ips_10_times_final.log
            fi
        done
	done
    done
done

