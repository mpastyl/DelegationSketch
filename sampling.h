#ifndef _SAMPLING
#define _SAMPLING

#include "relation.h"



using namespace std;


/*
Generic interface for sampling from a relation maintained as tuples.
*/

class Sampling
{
public:
	vector<unsigned int> *sample;

	Sampling();
	virtual ~Sampling();

	virtual void Build_Sample(Relation *rel) = 0;

	//dot product or size of join of two samples, and estimation
	virtual double Dot_Product(Sampling *s);
	virtual double Dot_Product_Estimation(Sampling *s) = 0;

	virtual double Scale_Dot_Product_Estimate(Sampling *s, double est) = 0;
};




/*
Bernoulli sampling with probability p
*/

class Bernoulli_Sampling : public Sampling
{
public:
	//p is the sampling probability; an element is sampled if its probability (randomly generated) is smaller than p; so, if p = 1.0, everyone is sampled, while for p = 0.0 nobody gets into the sample
	double p;

	Bernoulli_Sampling(double p);
	virtual ~Bernoulli_Sampling();

	virtual void Build_Sample(Relation *rel);

	virtual double Dot_Product_Estimation(Sampling *s);

	virtual double Scale_Dot_Product_Estimate(Sampling *s, double est);
};




/*
Sampling without replacement
*/

class WOR_Sampling : public Sampling
{
public:
	//sample size, population size
	int size, pop_size;

	WOR_Sampling(int size, int pop_size);
	virtual ~WOR_Sampling();

	virtual void Build_Sample(Relation *rel);

	virtual double Dot_Product_Estimation(Sampling *s);

	virtual double Scale_Dot_Product_Estimate(Sampling *s, double est);
};




/*
Sampling with replacement
*/

class WR_Sampling : public Sampling
{
public:
	//sample size, population size
	int size, pop_size;

	WR_Sampling(int size, int pop_size);
	virtual ~WR_Sampling();

	virtual void Build_Sample(Relation *rel);

	virtual double Dot_Product_Estimation(Sampling *s);

	virtual double Scale_Dot_Product_Estimate(Sampling *s, double est);
};


#endif

