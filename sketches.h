#ifndef _SKETCH
#define _SKETCH


#include "xis.h"
#include "params.h"

using namespace std;



/*
Generic interface for the sketches estimating size of joins and self-join sizes
For details on all the sketches implemented here, see the paper:
	1) "Statistical Analysis of Sketch Estimators" by F. Rusu and A. Dobra
*/

class Sketch
{
  public:
    double Average(double *x, int n);
    double Median(double *x, int n);
    double Min(double *x, int n);


    virtual ~Sketch();

    //reseting the sketch structure
    virtual void Clear_Sketch() = 0;

    //updating the sketch with the value corresponding to the given key
    virtual void Update_Sketch(unsigned int key, double func) = 0;

    //estimating the size of join of two sketches; the second sketch is passed in as s1
    virtual double Size_Of_Join(Sketch *s1) = 0;

    //estimating the self-join size or the second frequency moment
    virtual double Self_Join_Size() = 0;

    //virtual double Query_Sketch(unsigned int key) = 0;
};



/*
The original AGMS sketches proposed in the paper:
	1) "Tracking join and self-join sizes in limited storage" by N. Alon et al.
An array of rows_no * cols_no basic sketch estimators is used to compute the improved estimator. cols_no estimators from the same row are averaged, then the median of the rows_no average estimators is returned as the final estimate. sketch_elem is the array of basic estimators. xi_pm1 is an array of pseudo-random variables used to compute the basic sketch estimators. A pseudo-random variable is attached to each basic sketch estimator.
*/

class AGMS_Sketch : public Sketch
{
  protected:
    unsigned int rows_no;
    unsigned int cols_no;

    double *sketch_elem;

    Xi **xi_pm1;


  public:
    AGMS_Sketch(unsigned int cols_no, unsigned int rows_no, Xi **xi_pm1);
    virtual ~AGMS_Sketch();

    virtual void Clear_Sketch();

    virtual void Update_Sketch(unsigned int key, double func);

    virtual double Size_Of_Join(Sketch *s1);

    virtual double Self_Join_Size();
};



/*
Fast-AGMS sketches proposed in the paper:
	1) "Sketching streams through the net: distributed approximate query tracking" by G. Cormode and M. Garofalakis
rows_no basic estimators are computed by hashing the key values to an array with buckets_no elements. Then the median of the rows_no estimators is returned as the final estimator. sketch_elem is an array of rows_no * buckets_no counters. A key is initially hashed using the xi_bucket pseudo-random variable corresponding to the actual row, then the counter in that bucket is updated with the +/-1 random variable generated for the same key by xi_pm1 corresponding to the same row.
*/

class FAGMS_Sketch : public Sketch
{
  protected:
    unsigned int buckets_no;
    unsigned int rows_no;

    double *sketch_elem;

    Xi **xi_bucket;
    Xi **xi_pm1;


  public:
    FAGMS_Sketch(unsigned int buckets_no, unsigned int rows_no, Xi **xi_bucket, Xi **xi_pm1);
    virtual ~FAGMS_Sketch();

    virtual void Clear_Sketch();

    virtual void Update_Sketch(unsigned int key, double func);

    virtual double Size_Of_Join(Sketch *s1);

    virtual double Self_Join_Size();
};



/*
Fast-Count sketches proposed in the paper:
	1) "Tabulation-based 4-universal hashing with applications to second moment estimation" by M. Thorup and Y. Zhang
rows_no basic estimators are computed by hashing the key values to an array with buckets_no elements. Then the average of the rows_no estimators is returned as the final estimator. sketch_elem is an array of rows_no * buckets_no counters. A key is hashed using the xi_bucket pseudo-random variable corresponding to the actual row, then the counter in that bucket is updated with the value corresponding to the key.
*/

class Fast_Count_Sketch : public Sketch
{
  protected:
    unsigned int buckets_no;
    unsigned int rows_no;

    double *sketch_elem;

    Xi **xi_bucket;


  public:
    Fast_Count_Sketch(unsigned int buckets_no, unsigned int rows_no, Xi **xi_bucket);
    virtual ~Fast_Count_Sketch();

    virtual void Clear_Sketch();

    virtual void Update_Sketch(unsigned int key, double func);

    virtual double Size_Of_Join(Sketch *s1);

    virtual double Self_Join_Size();
};




/*
Count-Min sketches proposed in the paper:
	1) "An improved data stream summary: the count-min sketch and its applications" by G. Cormode and S. Muthukrishnan
rows_no basic estimators are computed by hashing the key values to an array with buckets_no elements. Then the minimum of the rows_no estimators is returned as the final estimator. sketch_elem is an array of rows_no * buckets_no counters. A key is hashed using the xi_bucket pseudo-random variable corresponding to the actual row, then the counter in that bucket is updated with the value corresponding to the key.
*/

class Count_Min_Sketch : public Sketch
{
  protected:
    unsigned int buckets_no;
    unsigned int rows_no;

    #if ATOMIC_INCREMENTS
    volatile int *sketch_elem;
    #else
    int *sketch_elem;
    #endif

    Xi **xi_bucket;


  public:
    Count_Min_Sketch(unsigned int buckets_no, unsigned int rows_no, Xi **xi_bucket);
    virtual ~Count_Min_Sketch();

    virtual void Clear_Sketch();

    virtual void Update_Sketch(unsigned int key, double func);
    
    double Query_Sketch(unsigned int key);

    virtual double Size_Of_Join(Sketch *s1);

    virtual double Self_Join_Size();
};


#endif
