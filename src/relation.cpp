#include "relation.h"
#include "data_generator.h"

#include <vector>


using namespace std;



Relation::Relation(unsigned int dom_size, unsigned int tuples_no)
{
  this->dom_size = dom_size;

  this->tuples_no = tuples_no;
  tuples = NULL;
}


Relation::~Relation()
{
  dom_size = 0;
  tuples_no = 0;

  delete tuples;
  tuples = NULL;
}


/*
The frequency of each value in the domain is first generated. Then the corresponding number of tuples is inserted in the tuples vector. The frequencies can be decorelated at different degrees: decor_param < 0 corresponds to negative correlation; decor_param = 0 corresponds to complete randomness; decor_param = 1 corresponds to complete positive correlation or identical relations.
*/

void Relation::Generate_Data(int type, double data_param, double decor_param)
{
  vector<unsigned int> freq(dom_size, 0);

  switch (type)
  {
    case 1:
    {
      tuples_no = generate_zipf(dom_size, tuples_no, data_param, freq);
      break;
    }
    case 2:
    {
      tuples_no = generate_reversed_zipf(dom_size, tuples_no, data_param, freq);
      break;
    }
    case 3:
    {
      tuples_no = generate_uniform(dom_size, tuples_no, freq);
      break;
    }
    case 4:
    {
      tuples_no = generate_uniform_limited(dom_size, tuples_no, data_param, freq);
      break;
    }
    case 5:
    {
      tuples_no = generate_zipf_right(dom_size, tuples_no, data_param, decor_param, freq);
      break;
    }
  }


  decorelate(freq, decor_param);


  tuples = new vector<unsigned int>(tuples_no, 0);

  int c_tup = 0;
  for (int i = 0; i < dom_size; i++)
  {
    for (int j = 0; j < freq[i]; j++)
      (*tuples)[c_tup + j] = i;

    c_tup += freq[i];
  }
}

