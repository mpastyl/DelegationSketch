#include "relation.h"
#include "xis.h"
#include "sketches.h"
#include "sampling.h"

#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>



using namespace std;


unsigned int Random_Generate()
{
  unsigned int x = rand();
  unsigned int h = rand();

  return x ^ ((h & 1) << 31);
}


int main(int argc, char** argv)
{
  int dom_size, tuples_no;
  int buckets_no, rows_no;

  int DIST_TYPE;
  double DIST_PARAM, DIST_SHUFF;

  int runs_no;

  double p;

  double sk_est, sk_samp_est;

  int i, j;


  if (argc != 10)
  {
	printf("Usage: sketch_sampling.out dom_size tuples_no buckets_no rows_no DIST_TYPE DIST_PARAM DIST_SHUFF SAMP_PROB runs_no\n");
	exit(1);
  }


  dom_size = atoi(argv[1]);
  tuples_no = atoi(argv[2]);

  buckets_no = atoi(argv[3]);
  rows_no = atoi(argv[4]);

  DIST_TYPE = atoi(argv[5]);
  DIST_PARAM = atof(argv[6]);
  DIST_SHUFF = atof(argv[7]);

  p = atof(argv[8]);

  runs_no = atoi(argv[9]);


  srand((unsigned int)time((time_t*)NULL));


  //generate the two relations
  Relation *r1 = new Relation(dom_size, tuples_no);
  Relation *r2 = new Relation(dom_size, tuples_no);

  r1->Generate_Data(DIST_TYPE, DIST_PARAM, 1.0);
  r2->Generate_Data(DIST_TYPE, DIST_PARAM, DIST_SHUFF);


  for (j = 0; j < runs_no; j++)
  {
	unsigned int I1, I2;


	//generate the pseudo-random numbers for FAGMS sketches; use EH3 and CW2B
	Xi **fagms_eh3 = new Xi*[rows_no];
	Xi **fagms_cw2b = new Xi*[rows_no];
	for (i = 0; i < rows_no; i++)
	{
		I1 = Random_Generate();
		I2 = Random_Generate();
		fagms_eh3[i] = new Xi_EH3(I1, I2);

		I1 = Random_Generate();
		I2 = Random_Generate();
		fagms_cw2b[i] = new Xi_CW2B(I1, I2, buckets_no);
	}

	//build the sketches
	Sketch *fagms1 = new FAGMS_Sketch(buckets_no, rows_no, fagms_cw2b, fagms_eh3);
	Sketch *fagms2 = new FAGMS_Sketch(buckets_no, rows_no, fagms_cw2b, fagms_eh3);


	//update the sketch for relation 1
	for (i = 0; i < r1->tuples_no; i++)
	{
		fagms1->Update_Sketch((*r1->tuples)[i], 1.0);
	}

	//update the sketch for relation 2
	for (i = 0; i < r2->tuples_no; i++)
	{
		fagms2->Update_Sketch((*r2->tuples)[i], 1.0);
	}

	//compute the sketch estimate
	sk_est = fagms1->Size_Of_Join(fagms2);



	//clear the sketches
	fagms1->Clear_Sketch();
	fagms2->Clear_Sketch();



	//build the Bernoulli samples for the two relations
	Sampling *s1 = new Bernoulli_Sampling(p);
	s1->Build_Sample(r1);

	Sampling *s2 = new Bernoulli_Sampling(p);
	s2->Build_Sample(r2);


	//update the sketch for sampled relation 1
	for (i = 0; i < s1->sample->size(); i++)
	{
		fagms1->Update_Sketch((*s1->sample)[i], 1.0);
	}

	//update the sketch for sampled relation 2
	for (i = 0; i < s2->sample->size(); i++)
	{
		fagms2->Update_Sketch((*s2->sample)[i], 1.0);
	}


	//compute the sketch over samples estimate
	sk_samp_est = fagms1->Size_Of_Join(fagms2);
	sk_samp_est = s1->Scale_Dot_Product_Estimate(s2, sk_samp_est);



	//clean-up everything
	for (i = 0; i < rows_no; i++)
	{
		delete fagms_eh3[i];
		delete fagms_cw2b[i];
	}
	delete [] fagms_eh3;
	delete [] fagms_cw2b;


	delete fagms1;
	delete fagms2;

	delete s1;
	delete s2;


	printf("%20.2f %20.2f\n", sk_est, sk_samp_est);
  }


  delete r1;
  delete r2;

  return 0;
}


