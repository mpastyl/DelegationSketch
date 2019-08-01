#include "relation.h"
#include "xis.h"
#include "sketches.h"
#include "utils.h"
#include "thread_data.h"
#include "thread_utils.h"
#include "filter.h"

#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <unistd.h>
#include <x86intrin.h>
#include <algorithm>
#include <random>

using namespace std;

FilterStruct * filterMatrix;

int tuples_no;

#if (!DURATION && DELEGATION_FILTERS) 
volatile int threadsFinished = 0;
#endif

unsigned int Random_Generate()
{
    unsigned int x = rand();
    unsigned int h = rand();

    return x ^ ((h & 1) << 31);
}


int shouldQuery(int index, int tid){
    return (index + tid)% 100; //NOTE: not random enough?
}

void serveDelegatedInserts(threadDataStruct * localThreadData){
    #if USE_LIST_OF_FILTERS
    // Check if needed?
    //if (!localThreadData->insertsPending) return;
    //printf("Filter List Head points to %p\n",localThreadData->listOfFullFilters);
    if (!localThreadData->listOfFullFilters) return;
    while (localThreadData->listOfFullFilters){
        FilterStruct* filter = pop(&(localThreadData->listOfFullFilters));
        // parse it and add each element to your own filter
        for (int i=0; i<FILTER_SIZE;i++){
            int key = filter->filter_id[i];
            unsigned int count = filter->filter_count[i];
            insertFilterNoWriteBack(localThreadData, key, count);
            // flush each element
            filter->filter_id[i] = -1;
            filter->filter_count[i] = 0;
        }
        // mark it as empty
        filter->filterCount = 0;
    }

    #else
    // Check if needed?
    if (!localThreadData->insertsPending) return;
    for (int thread=0; thread<numberOfThreads; thread++){
        // if Filter is full
        FilterStruct * filter  = &(filterMatrix[thread * numberOfThreads + localThreadData->tid]);
        if (filter->filterFull){
            // parse it and add each element to your own filter
            for (int i=0; i<FILTER_SIZE;i++){
                int key = filter->filter_id[i];
                unsigned int count = filter->filter_count[i];
                insertFilterNoWriteBack(localThreadData, key, count);
                // flush each element
                filter->filter_id[i] = -1;
                filter->filter_count[i] = 0;
            }
            // mark it as empty
            filter->filterCount = 0;
            filter->filterFull = 0;
        }
    }
    localThreadData->insertsPending = 0;
    #endif
}

void serveDelegatedQueries(threadDataStruct *localThreadData){
    if (!localThreadData->queriesPending) return;
    for (int i=0; i<numberOfThreads; i++){
        if (localThreadData->pendingQueriesFlags[i]){
            unsigned int countInFilter = queryFilter(localThreadData->pendingQueriesKeys[i], &(localThreadData->Filter));
            if (countInFilter){
                localThreadData->pendingQueriesCounts[i] = countInFilter;
            }
            else{
                localThreadData->pendingQueriesCounts[i] = localThreadData->theSketch->Query_Sketch(localThreadData->pendingQueriesKeys[i]);
            }
            //Also check the delegation filters
            for (int j =0; j < numberOfThreads; j++){
                localThreadData->pendingQueriesCounts[i] += queryFilter(localThreadData->pendingQueriesKeys[i], &(filterMatrix[j * numberOfThreads + localThreadData->tid]));
            }
            localThreadData->pendingQueriesFlags[i] = 0;
        }
    }
    localThreadData->queriesPending = 0;
}

void serveDelegatedInsertsAndQueries(threadDataStruct *localThreadData){
    serveDelegatedInserts(localThreadData);
    serveDelegatedQueries(localThreadData);
}

void delegateInsert(threadDataStruct * localThreadData, unsigned int key, unsigned int increment){
    int owner = key % numberOfThreads;
    FilterStruct * filter = &(filterMatrix[localThreadData->tid * numberOfThreads + owner]);
    threadDataStruct * owningThread = &(threadData[owner]);
    //try to insert in filterMatrix[localThreadData->tid * numberofThreads + owner]
    #if USE_LIST_OF_FILTERS
    while((!tryInsertInDelegatingFilterWithList(filter, key, owningThread)) && startBenchmark){   // I might deadlock if i am waiting for a thread that finished the benchmark
        //If it is full? Maybe try to serve your own pending requests and try again?
        //if (!threadData[owner].insertsPending) threadData[owner].insertsPending = 1;
        serveDelegatedInsertsAndQueries(localThreadData);
    }
    #else
    while((!tryInsertInDelegatingFilter(filter, key)) && startBenchmark){   // I might deadlock if i am waiting for a thread that finished the benchmark
        //If it is full? Maybe try to serve your own pending requests and try again?
        if (!threadData[owner].insertsPending) threadData[owner].insertsPending = 1;
        serveDelegatedInsertsAndQueries(localThreadData);
    }
    #endif
}

unsigned int delegateQuery(threadDataStruct * localThreadData, unsigned int key){
    int owner = key % numberOfThreads;
    while (threadData[owner].pendingQueriesFlags[localThreadData->tid]  && startBenchmark){
        serveDelegatedInsertsAndQueries(localThreadData);
    }

    threadData[owner].pendingQueriesKeys[localThreadData->tid] = key;
    threadData[owner].pendingQueriesCounts[localThreadData->tid] = 0;
    threadData[owner].pendingQueriesFlags[localThreadData->tid] = 1;

    if (!threadData[owner].queriesPending) threadData[owner].queriesPending = 1;
    while (threadData[owner].pendingQueriesFlags[localThreadData->tid] && startBenchmark){
        if (!threadData[owner].queriesPending) threadData[owner].queriesPending = 1;
        serveDelegatedInsertsAndQueries(localThreadData);
    }
    return threadData[owner].pendingQueriesCounts[localThreadData->tid];
}

double querry(threadDataStruct * localThreadData, unsigned int key){
    #if HYBRID
    double approximate_freq = localThreadData->theGlobalSketch->Query_Sketch(key);
    approximate_freq += (HYBRID-1)*numberOfThreads; //The amount of slack that can be hiden in the local copies
    #elif REMOTE_INSERTS || USE_MPSC
    double approximate_freq = localThreadData->sketchArray[key % numberOfThreads]->Query_Sketch(key);
    #elif LOCAL_COPIES
    double approximate_freq = 0;
    for (int j=0; j<numberOfThreads; j++){
        approximate_freq += localThreadData->sketchArray[j]->Query_Sketch(key);
    }
    #elif AUGMENTED_SKETCH   // WARNING: Queries are not thread safe right now
    double approximate_freq = 0;
    for (int j=0; j<numberOfThreads; j++){
        unsigned int countInFilter = queryFilter(key, &(localThreadData->Filter));
        if (countInFilter){
            approximate_freq += countInFilter;
        }
        else{
            approximate_freq += localThreadData->sketchArray[j]->Query_Sketch(key);
        }
    }
    #elif SHARED_SKETCH
    double approximate_freq = localThreadData->theGlobalSketch->Query_Sketch(key);
    #elif DELEGATION_FILTERS
    double approximate_freq = delegateQuery(localThreadData, key);
    #else 
        #error "Preprocessor flags not properly set"
    #endif
    #if USE_FILTER
    approximate_freq += (MAX_FILTER_SLACK-1)*numberOfThreads; //The amount of slack that can be hiden in the local copies
    #endif

    return approximate_freq;
}




void insert(threadDataStruct * localThreadData, unsigned int key, unsigned int increment){
#if USE_MPSC
    int owner = key % numberOfThreads; 
    localThreadData->sketchArray[owner]->enqueueRequest(key);
    localThreadData->theSketch->serveAllRequests(); //Serve any requests you can find in your own queue
#elif REMOTE_INSERTS
    int owner = key % numberOfThreads;
    localThreadData->sketchArray[owner]->Update_Sketch_Atomics(key, increment);
#elif HYBRID
    localThreadData->theSketch->Update_Sketch_Hybrid(key, 1.0, HYBRID);
#elif LOCAL_COPIES || AUGMENTED_SKETCH || DELEGATION_FILTERS
    localThreadData->theSketch->Update_Sketch(key, double(increment));
#elif SHARED_SKETCH
    localThreadData->theGlobalSketch->Update_Sketch_Atomics(key, increment);
#endif
}

void threadWork(threadDataStruct *localThreadData)
{
    //printf("Hello from thread %d\n", localThreadData->tid);
    int start = localThreadData->startIndex;
    int end = localThreadData->endIndex;
    int i;
    int numInserts = 0;
    int numQueries = 0;
    i = start;
    int elementsProcessed = 0;
    while (!startBenchmark)
    {
    }
    while (startBenchmark)
    {
        for (i = start; i < end; i++)
        {
            if (!startBenchmark)
            {
                break;
            }
            if (shouldQuery(i, localThreadData->tid) < QUERRY_RATE)
            {
                numQueries++;
                double approximate_freq = querry(localThreadData, i);
                localThreadData->returnData += approximate_freq;
            }
            numInserts++;
            #if DELEGATION_FILTERS
            serveDelegatedInserts(localThreadData);
            delegateInsert(localThreadData, (*localThreadData->theData->tuples)[i], 1);
            #elif AUGMENTED_SKETCH
            insertFilterNoWriteBack(localThreadData,(*localThreadData->theData->tuples)[i], 1);
            #elif USE_FILTER
            insertFilterWithWriteBack(localThreadData,(*localThreadData->theData->tuples)[i]);
            #else
            insert(localThreadData, (*localThreadData->theData->tuples)[i], 1);
            #endif
            localThreadData->elementsProcessed++;
        }
        //If duration is 0 then I only loop once over the input. This is to do accuracy tests.
        if (DURATION == 0){
            break;
        }
    }
    #if (!DURATION && DELEGATION_FILTERS) 
    // keep clearing your backlog, other wise we might endup in a deadlock
    serveDelegatedInsertsAndQueries(localThreadData); 
    __sync_fetch_and_add( &threadsFinished, 1);
    while( threadsFinished < numberOfThreads){
        serveDelegatedInsertsAndQueries(localThreadData); 
    }
    #endif
    localThreadData->numQueries = numQueries;
    localThreadData->numInserts = numInserts;
    // if (localThreadData->tid ==1){
    //     printFilter(localThreadData->Filter);
    // }
}


void * threadEntryPoint(void * threadArgs){
    int tid = *((int *) threadArgs);
    threadDataStruct * localThreadData = &(threadData[tid]);
    #if ITHACA
    //ITHACA: fill first NUMA node first (even numbers)
    int thread_id = (tid%36)*2 + tid/36;
    setaffinity_oncpu(thread_id);
    #else
    setaffinity_oncpu(14*(tid%2)+(tid/2)%14);
    #endif

    int threadWorkSize = tuples_no /  numberOfThreads;
    localThreadData->startIndex = tid * threadWorkSize;
    localThreadData->endIndex =  localThreadData->startIndex + threadWorkSize; //Stop before you reach that index

    for (int i=0; i<FILTER_SIZE; i++){
        localThreadData->Filter.filter_id[i] = -1;
        localThreadData->Filter.filter_count[i] = 0;
        localThreadData->Filter.filter_old_count[i] = 0;
    }
    localThreadData->Filter.filterCount = 0;

    localThreadData->pendingQueriesKeys =  (int *)calloc(numberOfThreads, sizeof(int));
    localThreadData->pendingQueriesCounts =  (unsigned int *)calloc(numberOfThreads, sizeof(unsigned int));
    localThreadData->pendingQueriesFlags =  (volatile int *)calloc(numberOfThreads, sizeof(volatile int));
    for(int i=0; i<numberOfThreads; i++){
        localThreadData->pendingQueriesKeys[i] = -1;
    }

    localThreadData->insertsPending = 0;
    localThreadData->queriesPending = 0;
    localThreadData->listOfFullFilters = NULL;

    barrier_cross(&barrier_global);
    barrier_cross(&barrier_started);

    threadWork(localThreadData);

    return NULL;
}

void postProcessing(){

    long int sumNumQueries=0, sumNumInserts = 0;
    double sumReturnValues = 0;
    for (int i=0; i<numberOfThreads; i++){
        sumNumQueries += threadData[i].numQueries;
        sumNumInserts += threadData[i].numInserts;
        sumReturnValues += threadData[i].returnData;
    }
    float percentage  = (float) sumNumQueries * 100/(sumNumQueries + sumNumInserts);
    printf("LOG: num Queries: %ld, num Inserts %ld, percentage %f garbage print %f\n",sumNumQueries, sumNumInserts, percentage, sumReturnValues);
}

int main(int argc, char **argv)
{
    int dom_size;
    int buckets_no, rows_no;

    int DIST_TYPE;
    double DIST_PARAM, DIST_SHUFF;

    int runs_no;

    double agms_est, fagms_est, fc_est, cm_est;

    int i, j;

    if (argc != 12)
    {
        printf("Usage: sketch_compare.out dom_size tuples_no buckets_no rows_no DIST_TYPE DIST_PARAM DIST_DECOR runs_no num_threads querry_rate duration(in sec, 0 means one pass over the data)\n");
        exit(1);
    }

    dom_size = atoi(argv[1]);
    tuples_no = atoi(argv[2]);

    buckets_no = atoi(argv[3]);
    rows_no = atoi(argv[4]);

    DIST_TYPE = atoi(argv[5]);
    DIST_PARAM = atof(argv[6]);
    DIST_SHUFF = atof(argv[7]);

    runs_no = atoi(argv[8]);

    numberOfThreads = atoi(argv[9]);
    QUERRY_RATE = atoi(argv[10]);

    DURATION = atoi(argv[11]);

    srand((unsigned int)time((time_t *)NULL));

    //Ground truth histrogram
    unsigned int *hist1 = (unsigned int *)calloc(dom_size, sizeof(unsigned int));
    unsigned long long true_join_size = 0;

    //generate the two relations
    Relation *r1 = new Relation(dom_size, tuples_no);

    r1->Generate_Data(DIST_TYPE, DIST_PARAM, DIST_SHUFF); //Note last arg should be 1
    if (r1->tuples_no < tuples_no){ //Sometimes Generate_Data might generate less than tuples_no
        tuples_no = r1->tuples_no;
    }
    auto rng = default_random_engine {};
    shuffle(begin((*r1->tuples)), end((*r1->tuples)), rng);

    for (j = 0; j < runs_no; j++)
    {
        unsigned int I1, I2;

        //generate the pseudo-random numbers for CM sketches; use CW2B
        //NOTE: doesn't work with CW2B, need to use CW4B. Why?
        Xi **cm_cw2b = new Xi *[rows_no];
        for (i = 0; i < rows_no; i++)
        {
            I1 = Random_Generate();
            I2 = Random_Generate();
            cm_cw2b[i] = new Xi_CW4B(I1, I2, buckets_no);
        }

        unsigned long long true_join_size = 0;
        for (i = 0; i < dom_size; i++)
        {
            hist1[i] = 0;
        }

        printf("size of the sketch %lu\n",sizeof(Count_Min_Sketch));
        globalSketch = new Count_Min_Sketch(buckets_no, rows_no, cm_cw2b);
        Count_Min_Sketch ** cmArray = (Count_Min_Sketch **) aligned_alloc(64, numberOfThreads * sizeof(Count_Min_Sketch *));
        for (i=0; i<numberOfThreads; i++){
            cmArray[i] = new Count_Min_Sketch(buckets_no, rows_no, cm_cw2b);
            cmArray[i]->SetGlobalSketch(globalSketch);
        }

        filterMatrix = (FilterStruct *) calloc(numberOfThreads*numberOfThreads, sizeof(FilterStruct));
        for (int thread = 0; thread< numberOfThreads * numberOfThreads; thread++){
            for (int j=0; j< FILTER_SIZE; j++){
                filterMatrix[thread].filter_id[j] = -1;
            }
        }

        for (i = 0; i < tuples_no; i++)
        {
            hist1[(*r1->tuples)[i]]++;
        }

        initThreadData(cmArray,r1);
        spawnThreads();
        barrier_cross(&barrier_global);        

        startTime();

        startBenchmark = 1;
        if (DURATION > 0){
            sleep(DURATION);
            startBenchmark = 0;
        }
        collectThreads();
        stopTime();

        postProcessing();

        printf("Total insertion time (ms): %lu\n",getTimeMs());
        long int totalElementsProcessed = 0;
        for (i=0; i<numberOfThreads; i++){
            totalElementsProcessed += threadData[i].elementsProcessed;
        }
        
        printf("Total processing throughput %f Mtouples per sec\n", (float)totalElementsProcessed / getTimeMs() / 1000);
        
        FILE *fp = fopen("logs/count_min_results.txt", "w");
        for (i = 0; i < dom_size; i++)
        {
            #if HYBRID
            double approximate_freq = globalSketch->Query_Sketch(i);
            approximate_freq += (HYBRID-1)*numberOfThreads; //The amount of slack that can be hiden in the local copies
            #elif REMOTE_INSERTS || USE_MPSC 
            double approximate_freq = cmArray[i % numberOfThreads]->Query_Sketch(i);
            #elif LOCAL_COPIES
            double approximate_freq = 0;
            for (int j=0; j<numberOfThreads; j++){
                approximate_freq += cmArray[j]->Query_Sketch(i);
            }
            #elif AUGMENTED_SKETCH   // WARNING: Queries are not thread safe right now
            double approximate_freq = 0;
            for (int j=0; j<numberOfThreads; j++){
                unsigned int countInFilter = queryFilter(i, &(threadData[j].Filter));
                if (countInFilter){
                    approximate_freq += countInFilter;
                }
                else{
                    approximate_freq += cmArray[j]->Query_Sketch(i);
                }
            }
            #elif SHARED_SKETCH
            double approximate_freq = globalSketch->Query_Sketch(i);
            #elif DELEGATION_FILTERS
            double approximate_freq = 0;
            int owner = i % numberOfThreads;
            unsigned int countInFilter = queryFilter(i, &(threadData[owner].Filter));
            if (countInFilter){
                approximate_freq += countInFilter;
            }
            else{
                approximate_freq += cmArray[owner]->Query_Sketch(i);
            }
            for (int j =0; j < numberOfThreads; j++){
                approximate_freq+= queryFilter(i, &(filterMatrix[j * numberOfThreads + owner]));
            }
            #endif
            #if USE_FILTER
                approximate_freq += (MAX_FILTER_SLACK-1)*numberOfThreads; //The amount of slack that can be hiden in the local copies
            #endif
            fprintf(fp, "%d %u %f\n", i, hist1[i], approximate_freq);
        }
        fclose(fp);
        //clean-up everything

        for (i = 0; i < rows_no; i++)
        {
            delete cm_cw2b[i];
        }

        delete[] cm_cw2b;

        for (i=0; i<numberOfThreads; i++){
            delete cmArray[i];
        }
        free(cmArray);
    
    printf("SHARED_SKETCH:       %d\n", SHARED_SKETCH);
    printf("LOCAL_COPIES:        %d\n", LOCAL_COPIES);
    printf("HYBRID:              %d\n", HYBRID);
    printf("REMOTE_INSERTS:      %d\n", REMOTE_INSERTS);
    printf("-----------------------\n");
    printf("DURATION:            %d\n", DURATION);
    printf("QUERRY RATE:         %d\n", QUERRY_RATE);
    printf("THREADS:             %d\n", numberOfThreads);

    }

    delete r1;

    return 0;
}