#include "relation.h"
#include "xis.h"
#include "sketches.h"

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

  double agms_est, fagms_est, fc_est, cm_est;

  int i, j;


  if (argc != 9)
  {
	printf("Usage: sketch_compare.out dom_size tuples_no buckets_no rows_no DIST_TYPE DIST_PARAM DIST_DECOR runs_no\n");
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


  srand((unsigned int)time((time_t*)NULL));


  //Ground truth histrogram
  unsigned int * hist1 = (unsigned int *) calloc( dom_size, sizeof(unsigned int));
  unsigned int * hist2 = (unsigned int *) calloc( dom_size, sizeof(unsigned int));
  unsigned long long true_join_size = 0;

  //generate the two relations
  Relation *r1 = new Relation(dom_size, tuples_no);
  Relation *r2 = new Relation(dom_size, tuples_no);

  r1->Generate_Data(DIST_TYPE, DIST_PARAM, 1.0);
  r2->Generate_Data(DIST_TYPE, DIST_PARAM, DIST_SHUFF);



  for (j = 0; j < runs_no; j++)
  {
	unsigned int I1, I2;




	//generate the pseudo-random numbers for AGMS sketches; use EH3
	Xi **agms_eh3 = new Xi*[buckets_no * rows_no];
	for (i = 0; i < buckets_no * rows_no; i++)
	{
		I1 = Random_Generate();
		I2 = Random_Generate();
		agms_eh3[i] = new Xi_EH3(I1, I2);
	}

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

	//generate the pseudo-random numbers for FC sketches; use CW4B
	Xi **fc_cw4b = new Xi*[rows_no];
	for (i = 0; i < rows_no; i++)
	{
		I1 = Random_Generate();
		I2 = Random_Generate();
		fc_cw4b[i] = new Xi_CW4B(I1, I2, buckets_no);
	}

	//generate the pseudo-random numbers for CM sketches; use CW2B
	Xi **cm_cw2b = new Xi*[rows_no];
	for (i = 0; i < rows_no; i++)
	{
		I1 = Random_Generate();
		I2 = Random_Generate();
		cm_cw2b[i] = new Xi_CW2B(I1, I2, buckets_no);
	}




	unsigned long long true_join_size = 0;
	for (i=0; i < dom_size; i++){
		hist1[i] = 0;
		hist2[i] = 0;
	}

	//build the sketches for each of the two relations
	Sketch *agms1 = new AGMS_Sketch(buckets_no, rows_no, agms_eh3);
	Sketch *agms2 = new AGMS_Sketch(buckets_no, rows_no, agms_eh3);

	Sketch *fagms1 = new FAGMS_Sketch(buckets_no, rows_no, fagms_cw2b, fagms_eh3);
	Sketch *fagms2 = new FAGMS_Sketch(buckets_no, rows_no, fagms_cw2b, fagms_eh3);

	Sketch *fc1 = new Fast_Count_Sketch(buckets_no, rows_no, fc_cw4b);
	Sketch *fc2 = new Fast_Count_Sketch(buckets_no, rows_no, fc_cw4b);

	Sketch *cm1 = new Count_Min_Sketch(buckets_no, rows_no, cm_cw2b);
	Sketch *cm2 = new Count_Min_Sketch(buckets_no, rows_no, cm_cw2b);



	//update the sketches for relation 1
	for (i = 0; i < r1->tuples_no; i++)
	{
		hist1[(*r1->tuples)[i]]++;
		agms1->Update_Sketch((*r1->tuples)[i], 1.0);
		fagms1->Update_Sketch((*r1->tuples)[i], 1.0);
		fc1->Update_Sketch((*r1->tuples)[i], 1.0);
		cm1->Update_Sketch((*r1->tuples)[i], 1.0);
	}

	//update the sketches for relation 2
	for (i = 0; i < r2->tuples_no; i++)
	{
		hist2[(*r2->tuples)[i]]++;
		agms2->Update_Sketch((*r2->tuples)[i], 1.0);
		fagms2->Update_Sketch((*r2->tuples)[i], 1.0);
		fc2->Update_Sketch((*r2->tuples)[i], 1.0);
		cm2->Update_Sketch((*r2->tuples)[i], 1.0);
	}


	//Compute ground truth
	true_join_size = 0;
	for (i=0; i < dom_size; i++){
		true_join_size += hist1[i] * hist2[i];
	}

	//compute the sketch estimate
	agms_est = agms1->Size_Of_Join(agms2);
	fagms_est = fagms1->Size_Of_Join(fagms2);
	fc_est = fc1->Size_Of_Join(fc2);
	cm_est = cm1->Size_Of_Join(cm2);


	FILE * fp = fopen("count_min_results.txt","w");
	for (i=0; i < dom_size; i++){
		double approximate_freq = ((Count_Min_Sketch *)cm1)->Query_Sketch(i);
		fprintf(fp,"%d %u %f\n", i, hist1[i],approximate_freq);	
	}
	fclose(fp);
	//clean-up everything
	for (i = 0; i < buckets_no * rows_no; i++)
		delete agms_eh3[i];
	delete [] agms_eh3;

	for (i = 0; i < rows_no; i++)
	{
		delete fagms_eh3[i];
		delete fagms_cw2b[i];

		delete fc_cw4b[i];

		delete cm_cw2b[i];
	}
	delete [] fagms_eh3;
	delete [] fagms_cw2b;

	delete [] fc_cw4b;

	delete [] cm_cw2b;


	delete agms1;
	delete agms2;

	delete fagms1;
	delete fagms2;

	delete fc1;
	delete fc2;

	delete cm1;
	delete cm2;



	printf("%20.2llu | %20.2f %20.2f %20.2f %20.2f\n",true_join_size, agms_est, fagms_est, fc_est, cm_est);
  }


  delete r1;
  delete r2;

  return 0;
}

