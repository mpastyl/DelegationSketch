CPP = g++ -O2 -Wno-deprecated -pthread -std=c++11
CPPDEBUG = g++ -O2 -Wno-deprecated -g
DEPS = thread_data.h thread_utils.h cm_benchmark.h barrier.h data_generator.h gen_scheme.h range_sum.h relation.h RM7_range_sum.h sampling.h sketches.h utils.h xis.h

all: cm_shared.out cm_local_copies.out cm_hybrid.out cm_remote_inserts.out


sketch_sampling:  relation.o xis.o sketches.o sampling.o sketch_sampling.o
	$(CPP) -lgsl -lgslcblas -o sketch_sampling.out relation.o xis.o sketches.o sampling.o sketch_sampling.o

sketch_sampling.o : sketch_sampling.cpp
	$(CPP) -c sketch_sampling.cpp


sketch_compare:  relation.o xis.o sketches.o sketch_compare.o
	$(CPP) -o sketch_compare.out relation.o xis.o sketches.o sketch_compare.o

sketch_compare.o : sketch_compare.cpp $(DEPS)
	$(CPP) -c sketch_compare.cpp

# cm_benchmark.o : cm_benchmark.cpp $(DEPS)
# 	$(CPP) -c cm_benchmark.cpp 

# cm_benchmark:  relation.o xis.o sketches.o cm_benchmark.o
# 	$(CPP) -o cm_benchmark.out relation.o xis.o sketches.o cm_benchmark.o

relation.o : relation.h relation.cpp data_generator.h
	$(CPP) -c relation.cpp

xis.o : xis.h xis.cpp gen_scheme.h range_sum.h RM7_range_sum.h
	$(CPP) -c xis.cpp

sketches.o : sketches.h sketches.cpp $(DEPS)
	$(CPP) -c sketches.cpp

sampling.o : sampling.h sampling.cpp
	$(CPP) -c sampling.cpp

cm_shared.o: cm_benchmark.cpp $(DEPS)
	$(CPP) -c cm_benchmark.cpp -D SHARED_SKETCH=1 -D LOCAL_COPIES=0 -D HYBRID=0 -D REMOTE_INSERTS=0

cm_local_copies.o: cm_benchmark.cpp $(DEPS)
	$(CPP) -c cm_benchmark.cpp -D SHARED_SKETCH=0 -D LOCAL_COPIES=1 -D HYBRID=0 -D REMOTE_INSERTS=0

cm_hybrid.o: cm_benchmark.cpp $(DEPS)
	$(CPP) -c cm_benchmark.cpp -D SHARED_SKETCH=0 -D LOCAL_COPIES=0 -D HYBRID=5 -D REMOTE_INSERTS=0

cm_remote_inserts.o: cm_benchmark.cpp $(DEPS)
	$(CPP) -c cm_benchmark.cpp -D SHARED_SKETCH=0 -D LOCAL_COPIES=0 -D HYBRID=0 -D REMOTE_INSERTS=1

cm_shared.out:  relation.o xis.o sketches.o cm_shared.o
	$(CPP) -o $@ relation.o xis.o sketches.o cm_benchmark.o 

cm_local_copies.out:  relation.o xis.o sketches.o cm_local_copies.o
	$(CPP) -o $@ relation.o xis.o sketches.o cm_benchmark.o 

cm_hybrid.out:  relation.o xis.o sketches.o cm_hybrid.o
	$(CPP) -o $@ relation.o xis.o sketches.o cm_benchmark.o 

cm_remote_inserts.out:  relation.o xis.o sketches.o cm_remote_inserts.o
	$(CPP) -o $@ relation.o xis.o sketches.o cm_benchmark.o 

clean:
	rm -f *.out
	rm -f *.o
	rm -f *~

