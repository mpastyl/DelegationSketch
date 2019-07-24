#include "sketches.h"
#include "xis.h"


#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;



/*
Sketch interface
*/

double Sketch::Average(double *x, int n)
{
  double sum = 0;
  for (int i = 0; i < n; i++)
    sum += x[i];

  sum = sum / (double)n;
  return sum;
}



double Sketch::Median(double *x, int n)
{
  if (n == 1)
    return x[0];

  if (n == 2)
    return (x[0] + x[1]) / 2;

  double *X = new double[n];
  for (int i = 0; i < n; i++)
    X[i] = x[i];

  // implement bubble sort
  bool rpt = true;
  while (rpt)
  {
    rpt = false;

    for (int i = 0; i < n - 1; i++)
      if (X[i] > X[i+1])
      {
        double t = X[i];
        X[i] = X[i + 1];
        X[i + 1] = t;
        rpt = true;
      }
  }

  double res;

  if (n%2 == 0)
    res = (X[n/2 - 1] + X[n/2]) / 2.0;
  else
    res = X[n/2];

  delete [] X;

  return res;
}



double Sketch::Min(double *x, int n)
{
  if (n == 1)
    return x[0];

  if (n == 2)
    return (x[0] <= x[1] ? x[0] : x[1]);

  double min = x[0];
  for (int i = 1; i < n; i++)
    if (x[i] < min)
      min = x[i];

  return min;
}



Sketch::~Sketch()
{
}




/*
AGMS sketches
*/

AGMS_Sketch::AGMS_Sketch(unsigned int cols_no, unsigned int rows_no, Xi **xi_pm1)
{
  this->rows_no = rows_no;
  this->cols_no = cols_no;

  this->xi_pm1 = xi_pm1;

  this->sketch_elem = new double[rows_no * cols_no];
  for (int i = 0; i < rows_no * cols_no; i++)
    this->sketch_elem[i] = 0.0;
}


AGMS_Sketch::~AGMS_Sketch()
{
  rows_no = 0;
  cols_no = 0;

  xi_pm1 = NULL;

  delete [] sketch_elem;
  sketch_elem = NULL;
}


void AGMS_Sketch::Clear_Sketch()
{
  for (int i = 0; i < rows_no * cols_no; i++)
    sketch_elem[i] = 0.0;
}


void AGMS_Sketch::Update_Sketch(unsigned int key, double func)
{
  for (int i = 0; i < rows_no * cols_no; i++)
    sketch_elem[i] = sketch_elem[i] + xi_pm1[i]->element(key) * func;
}


double AGMS_Sketch::Size_Of_Join(Sketch *s1)
{
  double *basic_est = new double[rows_no * cols_no];
  for (int i = 0; i < rows_no * cols_no; i++)
    basic_est[i] = sketch_elem[i] * ((AGMS_Sketch*)s1)->sketch_elem[i];

  double *avg_est = new double[rows_no];
  for (int i = 0; i < rows_no; i++)
    avg_est[i] = Average(basic_est + i * cols_no, cols_no);

  double result = Median(avg_est, rows_no);

  delete [] basic_est;
  delete [] avg_est;

  return result;
}


double AGMS_Sketch::Self_Join_Size()
{
  double *basic_est = new double[rows_no * cols_no];
  for (int i = 0; i < rows_no * cols_no; i++)
    basic_est[i] = sketch_elem[i] * sketch_elem[i];

  double *avg_est = new double[rows_no];
  for (int i = 0; i < rows_no; i++)
    avg_est[i] = Average(basic_est + i * cols_no, cols_no);

  double result = Median(avg_est, rows_no);

  delete [] basic_est;
  delete [] avg_est;

  return result;
}






/*
Fast-AGMS sketches
*/

FAGMS_Sketch::FAGMS_Sketch(unsigned int buckets_no, unsigned int rows_no, Xi **xi_bucket, Xi **xi_pm1)
{
  this->buckets_no = buckets_no;
  this->rows_no = rows_no;

  this->xi_bucket = xi_bucket;
  this->xi_pm1 = xi_pm1;

  this->sketch_elem = new double[buckets_no * rows_no];
  for (int i = 0; i < buckets_no * rows_no; i++)
    this->sketch_elem[i] = 0.0;
}


FAGMS_Sketch::~FAGMS_Sketch()
{
  buckets_no = 0;
  rows_no = 0;

  xi_bucket = NULL;
  xi_pm1 = NULL;

  delete [] sketch_elem;
  sketch_elem = NULL;
}


void FAGMS_Sketch::Clear_Sketch()
{
  for (int i = 0; i < buckets_no * rows_no; i++)
    sketch_elem[i] = 0.0;
}


void FAGMS_Sketch::Update_Sketch(unsigned int key, double func)
{
  for (int i = 0; i < rows_no; i++)
  {
    int bucket = (int)xi_bucket[i]->element(key);
    sketch_elem[i * buckets_no + bucket] = sketch_elem[i * buckets_no + bucket] + xi_pm1[i]->element(key) * func;
  }
}


double FAGMS_Sketch::Size_Of_Join(Sketch *s1)
{
  double *basic_est = new double[rows_no];
  for (int i = 0; i < rows_no; i++)
  {
    basic_est[i] = 0.0;
    for (int j = 0; j < buckets_no; j++)
      basic_est[i] = basic_est[i] + sketch_elem[i * buckets_no + j] * ((FAGMS_Sketch*)s1)->sketch_elem[i * buckets_no + j];
  }

  double result = Median(basic_est, rows_no);

  delete [] basic_est;

  return result;
}


double FAGMS_Sketch::Self_Join_Size()
{
  double *basic_est = new double[rows_no];
  for (int i = 0; i < rows_no; i++)
  {
    basic_est[i] = 0.0;
    for (int j = 0; j < buckets_no; j++)
      basic_est[i] = basic_est[i] + sketch_elem[i * buckets_no + j] * sketch_elem[i * buckets_no + j];
  }

  double result = Median(basic_est, rows_no);

  delete [] basic_est;

  return result;
}






/*
Fast-Count sketches
*/

Fast_Count_Sketch::Fast_Count_Sketch(unsigned int buckets_no, unsigned int rows_no, Xi **xi_bucket)
{
  this->buckets_no = buckets_no;
  this->rows_no = rows_no;

  this->xi_bucket = xi_bucket;

  this->sketch_elem = new double[buckets_no * rows_no];
  for (int i = 0; i < buckets_no * rows_no; i++)
    this->sketch_elem[i] = 0.0;
}


Fast_Count_Sketch::~Fast_Count_Sketch()
{
  buckets_no = 0;
  rows_no = 0;

  xi_bucket = NULL;

  delete [] sketch_elem;
  sketch_elem = NULL;
}


void Fast_Count_Sketch::Clear_Sketch()
{
  for (int i = 0; i < buckets_no * rows_no; i++)
    sketch_elem[i] = 0.0;
}


void Fast_Count_Sketch::Update_Sketch(unsigned int key, double func)
{
  for (int i = 0; i < rows_no; i++)
  {
    int bucket = (int)xi_bucket[i]->element(key);
    sketch_elem[i * buckets_no + bucket] = sketch_elem[i * buckets_no + bucket] + func;
  }
}


double Fast_Count_Sketch::Size_Of_Join(Sketch *s1)
{
  double *basic_est = new double[rows_no];
  for (int i = 0; i < rows_no; i++)
  {
    double L1 = 0.0;
    double L1p = 0.0;
    double L2 = 0.0;
    for (int j = 0; j < buckets_no; j++)
    {
      L1 = L1 + sketch_elem[i * buckets_no + j];
      L1p = L1p + ((Fast_Count_Sketch*)s1)->sketch_elem[i * buckets_no + j];
      L2 = L2 + sketch_elem[i * buckets_no + j] * ((Fast_Count_Sketch*)s1)->sketch_elem[i * buckets_no + j];
    }

    basic_est[i] = 1.0 / ((double)buckets_no - 1) * ((double)buckets_no * L2 - L1 * L1p);
  }

  double result = Average(basic_est, rows_no);

  delete [] basic_est;

  return result;
}


double Fast_Count_Sketch::Self_Join_Size()
{
  double *basic_est = new double[rows_no];
  for (int i = 0; i < rows_no; i++)
  {
    double L1 = 0.0;
    double L2 = 0.0;
    for (int j = 0; j < buckets_no; j++)
    {
      L1 = L1 + sketch_elem[i * buckets_no + j];
      L2 = L2 + sketch_elem[i * buckets_no + j] * sketch_elem[i * buckets_no + j];
    }

    basic_est[i] = 1.0 / ((double)buckets_no - 1) * ((double)buckets_no * L2 - L1 * L1);
  }

  double result = Average(basic_est, rows_no);

  delete [] basic_est;

  return result;
}





/*
Count-Min sketches
*/

Count_Min_Sketch::Count_Min_Sketch(unsigned int buckets_no, unsigned int rows_no, Xi **xi_bucket)
{
  this->buckets_no = buckets_no;
  this->rows_no = rows_no;

  this->xi_bucket = xi_bucket;

  //this->sketch_elem = (volatile int *)aligned_alloc(64, 64*sizeof(int)*(buckets_no * rows_no));
  //this->sketch_elem = (volatile int *)aligned_alloc(64, 4*sizeof(int)*(buckets_no * rows_no) + 64-(sizeof(int)*(buckets_no * rows_no))%64);
  //printf("size of counter array %lu \n",  sizeof(int)*(buckets_no * rows_no) + 64-(sizeof(int)*(buckets_no * rows_no))%64);
  this->sketch_elem = new volatile int[buckets_no * rows_no];
  for (int i = 0; i < buckets_no * rows_no; i++)
    this->sketch_elem[i] = 0.0;
  this->theGlobalSketch = NULL;

  this->queue = new moodycamel::ConcurrentQueue<int>;
}


Count_Min_Sketch::~Count_Min_Sketch()
{
  buckets_no = 0;
  rows_no = 0;

  xi_bucket = NULL;

  delete [] sketch_elem;
  sketch_elem = NULL;
}

void Count_Min_Sketch::enqueueRequest(unsigned int key){
    this->queue->enqueue(key);
}

void Count_Min_Sketch::serveAllRequests(){
    unsigned int item;
    while(this->queue->try_dequeue(item)){
        this->Update_Sketch(item, 1);
    }
}
void Count_Min_Sketch::SetGlobalSketch(Count_Min_Sketch * theGlobalSketch)
{
    this->theGlobalSketch = theGlobalSketch;
}

void Count_Min_Sketch::Clear_Sketch()
{
  for (int i = 0; i < buckets_no * rows_no; i++)
    sketch_elem[i] = 0.0;
}


void Count_Min_Sketch::Update_Sketch(unsigned int key, double func)
{
  for (int i = 0; i < rows_no; i++)
  {
    int bucket = (int)xi_bucket[i]->element(key);
    sketch_elem[i * buckets_no + bucket] = sketch_elem[i * buckets_no + bucket] + func;
  }
}

void Count_Min_Sketch::Update_Sketch_Atomics(unsigned int key, unsigned int func)
{
  for (int i = 0; i < rows_no; i++)
  {
    int bucket = (int)xi_bucket[i]->element(key);
    __sync_fetch_and_add(&sketch_elem[i * buckets_no + bucket], func); //FIXME: change all func to ints so I can use them in fetch_and_add
  }
}

void Count_Min_Sketch::Update_Sketch_Hybrid(unsigned int key, double func, int slack)
{
  for (int i = 0; i < rows_no; i++)
  {
    int bucket = (int)xi_bucket[i]->element(key);
    sketch_elem[i * buckets_no + bucket] = sketch_elem[i * buckets_no + bucket] + func;
    if (sketch_elem[i * buckets_no + bucket] == slack){
       sketch_elem[i * buckets_no + bucket] = 0;
       theGlobalSketch->incrementRawCounter(i * buckets_no + bucket, slack);
    }
  }
}

void Count_Min_Sketch::incrementRawCounter(unsigned int counterIndex, int amount){
   __sync_fetch_and_add(&sketch_elem[counterIndex], amount); 
}

double Count_Min_Sketch::Query_Sketch(unsigned int key)
{
  unsigned int min_count = UINT_MAX;
  for (int i = 0; i < rows_no; i++)
  {
    int bucket = (int)xi_bucket[i]->element(key);
    if (sketch_elem[i * buckets_no + bucket] < min_count)
    {
	min_count = sketch_elem[i * buckets_no + bucket];
    }
  }
  return min_count;
}


double Count_Min_Sketch::Size_Of_Join(Sketch *s1)
{
  double *basic_est = new double[rows_no];
  for (int i = 0; i < rows_no; i++)
  {
    basic_est[i] = 0.0;
    for (int j = 0; j < buckets_no; j++)
      basic_est[i] = basic_est[i] + sketch_elem[i * buckets_no + j] * ((Count_Min_Sketch*)s1)->sketch_elem[i * buckets_no + j];
  }

  double result = Min(basic_est, rows_no);

  delete [] basic_est;

  return result;
}


double Count_Min_Sketch::Self_Join_Size()
{
  double *basic_est = new double[rows_no];
  for (int i = 0; i < rows_no; i++)
  {
    basic_est[i] = 0.0;
    for (int j = 0; j < buckets_no; j++)
      basic_est[i] = basic_est[i] + sketch_elem[i * buckets_no + j] * sketch_elem[i * buckets_no + j];
  }

  double result = Min(basic_est, rows_no);

  delete [] basic_est;

  return result;
}



