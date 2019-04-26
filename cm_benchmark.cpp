#include "relation.h"
#include "xis.h"
#include "sketches.h"
#include "utils.h"

#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

using namespace std;

unsigned int Random_Generate()
{
    unsigned int x = rand();
    unsigned int h = rand();

    return x ^ ((h & 1) << 31);
}

int main(int argc, char **argv)
{
    int dom_size, tuples_no;
    int buckets_no, rows_no;

    int DIST_TYPE;
    double DIST_PARAM, DIST_SHUFF;

    int runs_no;
    int num_threads;

    double agms_est, fagms_est, fc_est, cm_est;

    int i, j;

    if (argc != 10)
    {
        printf("Usage: sketch_compare.out dom_size tuples_no buckets_no rows_no DIST_TYPE DIST_PARAM DIST_DECOR runs_no num_threads\n");
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

    num_threads = atoi(argv[9]);

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
        for (i = 0; i < r1->tuples_no; i++)
        {
            hist1[(*r1->tuples)[i]]++;
        }


        startTime();
        //update the sketches for relation 1
        //#pragma omp parallel for 
        for (i = 0; i < r1->tuples_no; i++)
        {
            cm1->Update_Sketch((*r1->tuples)[i], 1.0);
        }
        stopTime();

        printf("Total insertion time (ms): %lu\n",getTimeMs());

        FILE *fp = fopen("count_min_results.txt", "w");
        for (i = 0; i < dom_size; i++)
        {
            double approximate_freq = ((Count_Min_Sketch *)cm1)->Query_Sketch(i);
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

    }

    delete r1;

    return 0;
}