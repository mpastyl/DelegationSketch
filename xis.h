#ifndef _XIS
#define _XIS

/*
  Generating schemes for different types of random variables
    -> +/-1 random variables
    -> k-valued random variables
  Fast range-summable random variables
  For details see the papers:
	1) "Fast Range-Summable Random Variables for Efficient Aggregate Estimation" by F. Rusu and A. Dobra
	2) "Pseudo-Random Number Generation for Sketch-Based Estimations" by F. Rusu and A. Dobra
*/


using namespace std;



class Xi
{
  public:
    virtual double element(unsigned int j) = 0;
    virtual double interval_sum(unsigned int alpha, unsigned int beta) = 0;

    virtual ~Xi();
};


/*
+/-1 random variables that are 3-wise independent
*/

class Xi_BCH3 : public Xi
{
  protected:
    unsigned int seeds[2];

  public:
    Xi_BCH3(unsigned int I1, unsigned int I2);
    virtual ~Xi_BCH3();

    virtual double element(unsigned int j);
    virtual double interval_sum(unsigned int alpha, unsigned int beta);
};



class Xi_EH3 : public Xi
{
  protected:
    unsigned int seeds[2];

  public:
    Xi_EH3(unsigned int I1, unsigned int I2);
    virtual ~Xi_EH3();

    virtual double element(unsigned int j);
    virtual double interval_sum(unsigned int alpha, unsigned int beta);
};



class Xi_CW2 : public Xi
{
  protected:
    unsigned int seeds[2];

  public:
    Xi_CW2(unsigned int I1, unsigned int I2);
    virtual ~Xi_CW2();

    virtual double element(unsigned int j);
    virtual double interval_sum(unsigned int alpha, unsigned int beta);
};



/*
B-valued random variables that are 2-wise independent
*/

class Xi_CW2B : public Xi
{
  protected:
    unsigned int seeds[2];
    unsigned int buckets_no;

  public:
    Xi_CW2B(unsigned int I1, unsigned int I2, unsigned int B);
    virtual ~Xi_CW2B();

    virtual double element(unsigned int j);
    virtual double interval_sum(unsigned int alpha, unsigned int beta);
};



/*
+/-1 random variables that are 4-wise independent
*/

class Xi_CW4 : public Xi
{
  protected:
    unsigned int seeds[4];

  public:
    Xi_CW4(unsigned int I1, unsigned int I2);
    virtual ~Xi_CW4();

    virtual double element(unsigned int j);
    virtual double interval_sum(unsigned int alpha, unsigned int beta);
};



/*
B-valued random variables that are 4-wise independent
*/

class Xi_CW4B : public Xi
{
  protected:
    unsigned int seeds[4];
    unsigned int buckets_no;

  public:
    Xi_CW4B(unsigned int I1, unsigned int I2, unsigned int B);
    virtual ~Xi_CW4B();

    virtual double element(unsigned int j);
    virtual double interval_sum(unsigned int alpha, unsigned int beta);
};



/*
+/-1 random variables that are 5-wise independent
*/

class Xi_BCH5 : public Xi
{
  protected:
    unsigned int seeds[3];

  public:
    Xi_BCH5(unsigned int I1, unsigned int I2);
    virtual ~Xi_BCH5();

    virtual double element(unsigned int j);
    virtual double interval_sum(unsigned int alpha, unsigned int beta);
};



class Xi_RM7 : public Xi
{
  protected:
    unsigned int seeds[33];

  public:
    Xi_RM7(unsigned int I1, unsigned int I2);
    virtual ~Xi_RM7();

    virtual double element(unsigned int j);
    virtual double interval_sum(unsigned int alpha, unsigned int beta);
};





/*
Random variables for dyadic mapping as an alternative to fast range-summation
*/

class Xi_Dyadic_Map_EH3 : public Xi
{
  protected:
    unsigned int seeds[2];
    unsigned int dom_size;

  public:
    Xi_Dyadic_Map_EH3(unsigned int Dom_size, unsigned int I1, unsigned int I2);
    virtual ~Xi_Dyadic_Map_EH3();

    virtual double element(unsigned int j);
    virtual double interval_sum(unsigned int alpha, unsigned int beta);
};



class Xi_Dyadic_Map_BCH5 : public Xi
{
  protected:
    unsigned int seeds[3];
    unsigned int dom_size;

  public:
    Xi_Dyadic_Map_BCH5(unsigned int Dom_size, unsigned int I1, unsigned int I2);
    virtual ~Xi_Dyadic_Map_BCH5();

    virtual double element(unsigned int j);
    virtual double interval_sum(unsigned int alpha, unsigned int beta);
};


#endif
