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
    #if LOCAL_COPIES
    #if HYBRID
    double approximate_freq = ((Count_Min_Sketch *)localThreadData->theGlobalSketch)->Query_Sketch(key);
    approximate_freq += (HYBRID-1)*numberOfThreads; //The amount of slack that can be hiden in the local copies
    #else
    double approximate_freq = 0;
    for (int j=0; j<numberOfThreads; j++){
        approximate_freq += ((Count_Min_Sketch *)localThreadData->sketchArray[j])->Query_Sketch(key);
    }
    #endif
    #else
    double approximate_freq = ((Count_Min_Sketch *)localThreadData->theSketch)->Query_Sketch(key);
    #endif
    return approximate_freq;
}

void threadWork(threadDataStruct * localThreadData){
    //printf("Hello from thread %d\n", localThreadData->tid);
    int start =  localThreadData->startIndex;
    int end = localThreadData->endIndex;
    int i;
    int numInserts = 0;
    int numQueries = 0;
    #if FIXED_DURATION
    i =  start;
    int elementsProcessed = 0;
    while(!startBenchmark){}
    while (startBenchmark)
    #else
    for (i = start; i < end; i++)
    #endif
    {
        if (shouldQuery(i,localThreadData->tid) < QUERRY_RATE){
            numQueries++;
            double approximate_freq = querry(localThreadData, i);
            localThreadData->returnData += approximate_freq;
        }
        numInserts++;
        localThreadData->theSketch->Update_Sketch((*localThreadData->theData->tuples)[i], 1.0);
    #if FIXED_DURATION
    localThreadData->elementsProcessed++;
    i++;
    if (i==end) i = start;
    #endif
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

    if (argc != 11)
    {
        printf("Usage: sketch_compare.out dom_size tuples_no buckets_no rows_no DIST_TYPE DIST_PARAM DIST_DECOR runs_no num_threads querry_rate\n");
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

        Sketch *cm1 = new Count_Min_Sketch(buckets_no, rows_no, cm_cw2b);

        #if LOCAL_COPIES
        #if HYBRID
        globalSketch = new Count_Min_Sketch(buckets_no, rows_no, cm_cw2b);
        #endif
        Sketch ** cmArray = (Sketch **) calloc(numberOfThreads, sizeof(Sketch *));
        for (i=0; i<numberOfThreads; i++){
            cmArray[i] = new Count_Min_Sketch(buckets_no, rows_no, cm_cw2b);
            #if HYBRID
            ((Count_Min_Sketch *)cmArray[i])->SetGlobalSketch(globalSketch);
            #endif
        }
        #endif

        for (i = 0; i < r1->tuples_no; i++)
        {
            hist1[(*r1->tuples)[i]]++;
        }

        #if LOCAL_COPIES
        initThreadData(cmArray,r1);
        #else
        initThreadData(cm1, r1);
        #endif
        spawnThreads();
        barrier_cross(&barrier_global);        

        startTime();
        //update the sketches for relation 1
        // for (i = 0; i < r1->tuples_no; i++)
        // {
        //     cm1->Update_Sketch((*r1->tuples)[i], 1.0);
        // }
        #if FIXED_DURATION
        startBenchmark = 1;
        sleep(FIXED_DURATION);
        startBenchmark = 0;
        #endif

        collectThreads();
        stopTime();

        postProcessing();

        printf("Total insertion time (ms): %lu\n",getTimeMs());
        #if FIXED_DURATION
        int totalElementsProcessed = 0;
        for (i=0; i<numberOfThreads; i++){
            totalElementsProcessed += threadData[i].elementsProcessed;
        }
        printf("Total processing throughput %f Mtouples per sec\n", (float)totalElementsProcessed / FIXED_DURATION / 1000000);
        #endif

        FILE *fp = fopen("count_min_results.txt", "w");
        for (i = 0; i < dom_size; i++)
        {
            #if LOCAL_COPIES
            #if HYBRID
            double approximate_freq = ((Count_Min_Sketch *)globalSketch)->Query_Sketch(i);
            approximate_freq += (HYBRID-1)*numberOfThreads; //The amount of slack that can be hiden in the local copies
            #else
            double approximate_freq = 0;
            for (int j=0; j<numberOfThreads; j++){
                approximate_freq += ((Count_Min_Sketch *)cmArray[j])->Query_Sketch(i);
            }
            #endif
            #else
            double approximate_freq = ((Count_Min_Sketch *)cm1)->Query_Sketch(i);
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

        delete cm1;
        #if LOCAL_COPIES
        for (i=0; i<numberOfThreads; i++){
            delete cmArray[i];
        }
        free(cmArray);
        #endif
    
    printf("UPDATE_ONLY_MINIMUM: %d\n", UPDATE_ONLY_MINIMUM);
    printf("ATOMIC_INCREMENTS:   %d\n", ATOMIC_INCREMENTS);
    printf("LOCAL_COPIES:        %d\n", LOCAL_COPIES);
    printf("QUERRY RATE:         %d\n", QUERRY_RATE);
    printf("FIXED DURATION:      %d\n", FIXED_DURATION);
    printf("HYBRID:              %d\n", HYBRID);

    }

    delete r1;

    return 0;
}