#include "sampling.h"

#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <gsl/gsl_rng.h>


using namespace std;



Sampling::Sampling()
{
	sample = new vector<unsigned int>;
}


Sampling::~Sampling()
{
	delete sample;
}


double Sampling::Dot_Product(Sampling *s)
{
	double res = 0.0;

	//first sort the tuples in order to aggregate for getting the frequency representation
	vector<unsigned int> aux_s1(*sample);
	vector<unsigned int> aux_s2(*s->sample);

	sort(aux_s1.begin(), aux_s1.end());
	sort(aux_s2.begin(), aux_s2.end());

	//go over the sorted vectors and build the key and frequency vectors
	vector<unsigned int> key_s1;
	vector<unsigned int> key_s2;

	vector<unsigned int> freq_s1;
	vector<unsigned int> freq_s2;

	int n, m, i, j, key, freq;

	n = aux_s1.size(), i = 0;
	while (i < n)
	{
		key = aux_s1[i];

		freq = 0;
		while ((i < n) && (aux_s1[i] == key))
		{
			freq += 1;
			i += 1;
		}

		key_s1.push_back(key);
		freq_s1.push_back(freq);
	}

	n = aux_s2.size(), i = 0;
	while (i < n)
	{
		key = aux_s2[i];

		freq = 0;
		while ((i < n) && (aux_s2[i] == key))
		{
			freq += 1;
			i += 1;
		}

		key_s2.push_back(key);
		freq_s2.push_back(freq);
	}


	//do the interclasation of the two sorted frequency vectors
	i = 0, j = 0, n = key_s1.size(), m = key_s2.size();
	while ((i < n) && (j < m))
	{
		if (key_s1[i] == key_s2[j])
		{
			res += (double)freq_s1[i] * (double)freq_s2[j];
			i += 1;
			j += 1;
		}
		else if (key_s1[i] < key_s2[j])
		{
			i += 1;
		}
		else if (key_s1[i] > key_s2[j])
		{
			j += 1;
		}
	}


	return res;
}





Bernoulli_Sampling::Bernoulli_Sampling(double p)
: Sampling()
{
	this->p = p;
}


Bernoulli_Sampling::~Bernoulli_Sampling()
{
}


void Bernoulli_Sampling::Build_Sample(Relation *rel)
{
	const gsl_rng_type *T;
	gsl_rng *r;

	int i;
	double pp;

	if (p == 0.0)
		return;

	gsl_rng_env_setup();
	
	T = gsl_rng_default;
	r = gsl_rng_alloc (T);
	gsl_rng_set(r, rand());
	
	for (i = 0; i < rel->tuples_no; i++)
	{
		if (p == 1.0)
		{
			sample->push_back((*rel->tuples)[i]);
		}
		else
		{
			pp = gsl_rng_uniform(r);
			if (pp < p)
			{
				sample->push_back((*rel->tuples)[i]);
			}
		}
	}

	gsl_rng_free (r);
}


double Bernoulli_Sampling::Dot_Product_Estimation(Sampling *s)
{
	double res = Dot_Product(s);

	Bernoulli_Sampling *aux_s = (Bernoulli_Sampling*)s;
	res *= (1.0 / p) * (1.0 / aux_s->p);

	return res;
}


double Bernoulli_Sampling::Scale_Dot_Product_Estimate(Sampling *s, double est)
{
	Bernoulli_Sampling *aux_s = (Bernoulli_Sampling*)s;
	double res = (1.0 / p) * (1.0 / aux_s->p) * est;

	return res;
}




WOR_Sampling::WOR_Sampling(int size, int pop_size)
: Sampling()
{
	this->size = size;
	this->pop_size = pop_size;
}


WOR_Sampling::~WOR_Sampling()
{
}


void WOR_Sampling::Build_Sample(Relation *rel)
{
	const gsl_rng_type *T;
	gsl_rng *r;

	int i, position;

	if (size <= 0)
		return;

	if (size >= rel->tuples_no)
	{
		for (i = 0; i < rel->tuples_no; i++)
		{
			sample->push_back((*rel->tuples)[i]);
		}
		
		return;
	}

	gsl_rng_env_setup();
	
	T = gsl_rng_default;
	r = gsl_rng_alloc (T);
	gsl_rng_set(r, rand());
	

	vector<unsigned int> *copy = new vector<unsigned int>(*rel->tuples);

	for (i = 0; i < size; i++)
	{
		position = gsl_rng_uniform_int (r, rel->tuples_no - i);

		sample->push_back((*copy)[position]);
		copy->erase (copy->begin() + position);
	}

	delete copy;


	gsl_rng_free (r);
}


double WOR_Sampling::Dot_Product_Estimation(Sampling *s)
{
	double res = Dot_Product(s);

	WOR_Sampling *aux_s = (WOR_Sampling*)s;
	res *= ((double)pop_size / (double)size) * ((double)aux_s->pop_size / (double)aux_s->size);

	return res;
}


double WOR_Sampling::Scale_Dot_Product_Estimate(Sampling *s, double est)
{
	WOR_Sampling *aux_s = (WOR_Sampling*)s;
	double res = ((double)pop_size / (double)size) * ((double)aux_s->pop_size / (double)aux_s->size) * est;

	return res;
}




WR_Sampling::WR_Sampling(int size, int pop_size)
: Sampling()
{
	this->size = size;
	this->pop_size = pop_size;
}


WR_Sampling::~WR_Sampling()
{
}


void WR_Sampling::Build_Sample(Relation *rel)
{
	const gsl_rng_type *T;
	gsl_rng *r;

	int i, position;

	if (size <= 0)
		return;

	gsl_rng_env_setup();
	
	T = gsl_rng_default;
	r = gsl_rng_alloc (T);
	gsl_rng_set(r, rand());
	

	for (i = 0; i < size; i++)
	{
		position = gsl_rng_uniform_int (r, rel->tuples_no);
		sample->push_back((*rel->tuples)[position]);
	}

	gsl_rng_free (r);
}


double WR_Sampling::Dot_Product_Estimation(Sampling *s)
{
	double res = Dot_Product(s);

	WR_Sampling *aux_s = (WR_Sampling*)s;
	res *= ((double)pop_size / (double)size) * ((double)aux_s->pop_size / (double)aux_s->size);

	return res;
}


double WR_Sampling::Scale_Dot_Product_Estimate(Sampling *s, double est)
{
	WR_Sampling *aux_s = (WR_Sampling*)s;
	double res = ((double)pop_size / (double)size) * ((double)aux_s->pop_size / (double)aux_s->size) * est;

	return res;
}


