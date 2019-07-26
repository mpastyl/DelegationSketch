INCLUDE = -I. -I../include
CPP = g++ -O2 -Wno-deprecated -pthread -std=c++11 $(INCLUDE) -DITHACA=$(ITHACA)
CPPDEBUG = g++ -O2 -Wno-deprecated -g
DEPS = thread_data.h thread_utils.h cm_benchmark.h barrier.h data_generator.h gen_scheme.h range_sum.h relation.h RM7_range_sum.h sampling.h sketches.h utils.h xis.h
BIN = ../bin

all: cm_shared.out cm_local_copies.out cm_hybrid.out cm_remote_inserts.out cm_remote_inserts_mpsc.out cm_remote_inserts_filtered.out cm_shared_filtered.out cm_local_copies_filtered.out cm_augmented_sketch.out cm_delegation_filters.out


sketch_sampling:  relation.o xis.o sketches.o sampling.o sketch_sampling.o
	$(CPP) -lgsl -lgslcblas -o sketch_sampling.out relation.o xis.o sketches.o sampling.o sketch_sampling.o

sketch_sampling.o : sketch_sampling.cpp
	$(CPP) -c sketch_sampling.cpp


sketch_compare:  relation.o xis.o sketches.o sketch_compare.o
	$(CPP) -o sketch_compare.out relation.o xis.o sketches.o sketch_compare.o

sketch_compare.o : sketch_compare.cpp 
	$(CPP) -c sketch_compare.cpp

# cm_benchmark.o : cm_benchmark.cpp $(DEPS)
# 	$(CPP) -c cm_benchmark.cpp 

# cm_benchmark:  relation.o xis.o sketches.o cm_benchmark.o
# 	$(CPP) -o cm_benchmark.out relation.o xis.o sketches.o cm_benchmark.o

relation.o : relation.cpp
	$(CPP) -c relation.cpp

xis.o : xis.cpp 
	$(CPP) -c xis.cpp

sketches.o : sketches.cpp 
	$(CPP) -c sketches.cpp

sampling.o : sampling.cpp
	$(CPP) -c sampling.cpp

cm_shared.o: cm_benchmark.cpp 
	$(CPP) -c cm_benchmark.cpp -D SHARED_SKETCH=1 

cm_local_copies.o: cm_benchmark.cpp
	$(CPP) -c cm_benchmark.cpp -D LOCAL_COPIES=1 

cm_hybrid.o: cm_benchmark.cpp
	$(CPP) -c cm_benchmark.cpp -D HYBRID=5 

cm_remote_inserts.o: cm_benchmark.cpp
	$(CPP) -c cm_benchmark.cpp -D REMOTE_INSERTS=1 

cm_remote_inserts_mpsc.o: cm_benchmark.cpp
	$(CPP) -c cm_benchmark.cpp -D USE_MPSC=1 

#Filtered versions
cm_remote_inserts_filtered.o: cm_benchmark.cpp
	$(CPP) -c cm_benchmark.cpp -D REMOTE_INSERTS=1 -D USE_FILTER=1

cm_shared_filtered.o: cm_benchmark.cpp 
	$(CPP) -c cm_benchmark.cpp -D SHARED_SKETCH=1 -D USE_FILTER=1

cm_local_copies_filtered.o: cm_benchmark.cpp 
	$(CPP) -c cm_benchmark.cpp -D LOCAL_COPIES=1 -D USE_FILTER=1

cm_augmented_sketch.o: cm_benchmark.cpp 
	$(CPP) -c cm_benchmark.cpp -D AUGMENTED_SKETCH=1

cm_delegation_filters.o: cm_benchmark.cpp 
	$(CPP) -c cm_benchmark.cpp -D DELEGATION_FILTERS=1
#--------------------
cm_shared.out:  relation.o xis.o sketches.o cm_shared.o
	$(CPP) -o ${BIN}/$@ relation.o xis.o sketches.o cm_benchmark.o 

cm_local_copies.out:  relation.o xis.o sketches.o cm_local_copies.o
	$(CPP) -o ${BIN}/$@ relation.o xis.o sketches.o cm_benchmark.o 

cm_hybrid.out:  relation.o xis.o sketches.o cm_hybrid.o
	$(CPP) -o ${BIN}/$@ relation.o xis.o sketches.o cm_benchmark.o 

cm_remote_inserts.out:  relation.o xis.o sketches.o cm_remote_inserts.o
	$(CPP) -o ${BIN}/$@ relation.o xis.o sketches.o cm_benchmark.o 

cm_remote_inserts_mpsc.out:  relation.o xis.o sketches.o cm_remote_inserts_mpsc.o
	$(CPP) -o ${BIN}/$@ relation.o xis.o sketches.o cm_benchmark.o 

#Filtered versions
cm_remote_inserts_filtered.out:  relation.o xis.o sketches.o cm_remote_inserts_filtered.o
	$(CPP) -o ${BIN}/$@ relation.o xis.o sketches.o cm_benchmark.o 

cm_shared_filtered.out:  relation.o xis.o sketches.o cm_shared_filtered.o
	$(CPP) -o ${BIN}/$@ relation.o xis.o sketches.o cm_benchmark.o 

cm_local_copies_filtered.out:  relation.o xis.o sketches.o cm_local_copies_filtered.o
	$(CPP) -o ${BIN}/$@ relation.o xis.o sketches.o cm_benchmark.o 

cm_augmented_sketch.out:  relation.o xis.o sketches.o cm_augmented_sketch.o
	$(CPP) -o ${BIN}/$@ relation.o xis.o sketches.o cm_benchmark.o 

cm_delegation_filters.out:  relation.o xis.o sketches.o cm_delegation_filters.o
	$(CPP) -o ${BIN}/$@ relation.o xis.o sketches.o cm_benchmark.o 

clean:
	rm -f *.out
	rm -f *.o
	rm -f *~
	rm -f ../bin/*

