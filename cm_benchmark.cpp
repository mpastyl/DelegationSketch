#include "relation.h"
#include "xis.h"
#include "sketches.h"
#include "utils.h"
#include "thread_data.h"
#include "thread_utils.h"

#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <unistd.h>

using namespace std;

int tuples_no;

unsigned int Random_Generate()
{
    unsigned int x = rand();
    unsigned int h = rand();

    return x ^ ((h & 1) << 31);
}

int shouldQuery(int index, int tid){
    return (index + tid)% 100; //NOTE: not random enough?
}

double querry(threadDataStruct * localThreadData, unsigned int key){
    #if HYBRID
    double approximate_freq = ((Count_Min_Sketch *)localThreadData->theGlobalSketch)->Query_Sketch(key);
    approximate_freq += (HYBRID-1)*numberOfThreads; //The amount of slack that can be hiden in the local copies
    #elif REMOTE_INSERTS
    double approximate_freq = ((Count_Min_Sketch *)localThreadData->sketchArray[key % numberOfThreads])->Query_Sketch(key);
    #elif LOCAL_COPIES
    double approximate_freq = 0;
    for (int j=0; j<numberOfThreads; j++){
        approximate_freq += ((Count_Min_Sketch *)localThreadData->sketchArray[j])->Query_Sketch(key);
    }
    #elif SHARED_SKETCH
    double approximate_freq = ((Count_Min_Sketch *)localThreadData->theGlobalSketch)->Query_Sketch(key);
    #else 
        #error "Preprocessor flags not properly set"
    #endif
    return approximate_freq;
}

void insert(threadDataStruct * localThreadData, unsigned int key){
    #if REMOTE_INSERTS
    int owner = key % numberOfThreads; 
    ((Count_Min_Sketch *)localThreadData->sketchArray[owner])->Update_Sketch_Atomics(key, 1.0);
    #elif HYBRID
    ((Count_Min_Sketch *)localThreadData->theSketch)->Update_Sketch_Hybrid(key, 1.0, HYBRID);
    #elif LOCAL_COPIES
    localThreadData->theSketch->Update_Sketch(key, 1.0);
    #elif SHARED_SKETCH
    ((Count_Min_Sketch *)localThreadData->theGlobalSketch)->Update_Sketch_Atomics(key, 1.0);
    #endif
}

void threadWork(threadDataStruct * localThreadData){
    //printf("Hello from thread %d\n", localThreadData->tid);
    int start =  localThreadData->startIndex;
    int end = localThreadData->endIndex;
    int i;
    int numInserts = 0;
    int numQueries = 0;
    i =  start;
    int elementsProcessed = 0;
    while(!startBenchmark){}
    while (startBenchmark){
        for (i = start; i < end; i++){
            if (!startBenchmark){
                break;
            }
            if (shouldQuery(i,localThreadData->tid) < QUERRY_RATE){
                numQueries++;
                double approximate_freq = querry(localThreadData, i);
                localThreadData->returnData += approximate_freq;
            }
            numInserts++;
            insert(localThreadData, (*localThreadData->theData->tuples)[i]);
            localThreadData->elementsProcessed++;
        }
        //If duration is 0 then I only loop once over the input. This is to do accuracy tests.
        if (DURATION == 0){
            break;
        }
    }
    localThreadData->numQueries = numQueries;
    localThreadData->numInserts = numInserts;
}


void * threadEntryPoint(void * threadArgs){
    int tid = *((int *) threadArgs);
    threadDataStruct * localThreadData = &(threadData[tid]);
    //setaffinity_oncpu(14*(tid%2)+(tid/2)%14);
    setaffinity_oncpu(tid*2);

    int threadWorkSize = tuples_no /  numberOfThreads;
    localThreadData->startIndex = tid * threadWorkSize;
    localThreadData->endIndex =  localThreadData->startIndex + threadWorkSize; //Stop before you reach that index

    barrier_cross(&barrier_global);
    barrier_cross(&barrier_started);

    threadWork(localThreadData);

    return NULL;
}

void postProcessing(){

    int sumNumQueries, sumNumInserts = 0;
    double sumReturnValues = 0;
    for (int i=0; i<numberOfThreads; i++){
        sumNumQueries += threadData[i].numQueries;
        sumNumInserts += threadData[i].numInserts;
        sumReturnValues += threadData[i].returnData;
    }
    float percentage  = (float) sumNumQueries * 100/(sumNumQueries + sumNumInserts);
    printf("LOG: num Queries: %d, num Inserts %d, percentage %f garbage print %f\n",sumNumQueries, sumNumInserts, percentage, sumReturnValues);
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

    r1->Generate_Data(DIST_TYPE, DIST_PARAM, 1.0); //Note last arg should be 1

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


        globalSketch = new Count_Min_Sketch(buckets_no, rows_no, cm_cw2b);
        Sketch ** cmArray = (Sketch **) calloc(numberOfThreads, sizeof(Sketch *));
        for (i=0; i<numberOfThreads; i++){
            cmArray[i] = new Count_Min_Sketch(buckets_no, rows_no, cm_cw2b);
            ((Count_Min_Sketch *)cmArray[i])->SetGlobalSketch(globalSketch);
        }

        for (i = 0; i < r1->tuples_no; i++)
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
        int totalElementsProcessed = 0;
        for (i=0; i<numberOfThreads; i++){
            totalElementsProcessed += threadData[i].elementsProcessed;
        }
        if (DURATION > 0){
            printf("Total processing throughput %f Mtouples per sec\n", (float)totalElementsProcessed / DURATION / 1000000);
        }
        else{
            printf("Total processing throughput %f Mtouples per sec\n", (float)totalElementsProcessed / getTimeMs() / 1000000);
        }
        FILE *fp = fopen("count_min_results.txt", "w");
        for (i = 0; i < dom_size; i++)
        {
            #if HYBRID
            double approximate_freq = ((Count_Min_Sketch *)globalSketch)->Query_Sketch(i);
            approximate_freq += (HYBRID-1)*numberOfThreads; //The amount of slack that can be hiden in the local copies
            #elif REMOTE_INSERTS
            double approximate_freq = ((Count_Min_Sketch *)cmArray[i % numberOfThreads])->Query_Sketch(i);
            #elif LOCAL_COPIES
            double approximate_freq = 0;
            for (int j=0; j<numberOfThreads; j++){
                approximate_freq += ((Count_Min_Sketch *)cmArray[j])->Query_Sketch(i);
            }
            #elif SHARED_SKETCH
            double approximate_freq = ((Count_Min_Sketch *)globalSketch)->Query_Sketch(i);
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

    }

    delete r1;

    return 0;
}