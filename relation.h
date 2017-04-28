#ifndef _RELATION
#define _RELATION

#include <vector>


using namespace std;


/*
Class to store a basic relation
The domain of the possible values tuples can take is specified by dom_size. tuples_no is the number of tuples the relation contains, while tuples contains the actual values.
*/

class Relation
{
  public:
    unsigned int dom_size;

    unsigned int tuples_no;
    vector<unsigned int> *tuples;


    Relation(unsigned int dom_size, unsigned int tuples_no);
    virtual ~Relation();

    void Generate_Data(int type, double data_param, double decor_param);
};

#endif
