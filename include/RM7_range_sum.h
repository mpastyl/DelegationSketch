#ifndef _RM7_RANGE
#define _RM7_RANGE

#include <stdio.h>

/*
  Fast range-summation algorithm for RM7 scheme
  Although theoretically valuable, it has poor practical performance
  This is an implementation of the algorithms presented in the papers:
	1) "The computational complexity of (xor-and) counting problems" by A. Ehrenfeucht and M. Karpinski
	2) "Improved range-summable random variable construction algorithms" by A. R. Calderbank et al.

  Assumption: integers are represented on 32 bits
*/


struct Term
{
  int n;
  int *fact;
}; 

struct XOR_AND
{
  int n;
  Term *term;
};

struct XOR_AND_Expression
{
  int var_no;
  XOR_AND *exp;
};


struct Vector
{
  int n;
  int *elem;
};

struct Matrix
{
  int r, c;
  int **elem;
};


struct Data_Structure
{
  Matrix *t;
  Vector *c;
  int z;

  int type;
  int unique_elem;

  int beta_length;
  int **beta_form;
};


double Interval(unsigned int a, unsigned int b, unsigned int *S, unsigned int s0);
int Simplify_Expression(XOR_AND_Expression *output, unsigned int base, int p);
int Simplify_Term(XOR_AND_Expression *output, int n, int t_val);

int XOR_AND_2_EXACT_COUNT(XOR_AND_Expression *input, Data_Structure *output);

int Base_Expression(XOR_AND_Expression *expr, Data_Structure *data);
int Expression_Is_One(XOR_AND_Expression *expr);

int Linear_Independence(Data_Structure *beta, XOR_AND_Expression *alfa, Vector *coef);
int Gauss_Elimination(int n, int m, Matrix A, Vector b, Vector *x);

int Find_Factor(XOR_AND_Expression *expr);
int Factorize_Term(Term *t, int fact, Term *out);
int Factorize_Expression(XOR_AND_Expression *expr, int fact, XOR_AND_Expression *alfa, XOR_AND_Expression *beta);
int Factorize(XOR_AND_Expression *expr, int &factor, XOR_AND_Expression *alfa, XOR_AND_Expression *beta);

int Free_Expression(XOR_AND_Expression *e);
int Free_Data_Structure(Data_Structure *d);

int Copy_Term(Term *dest, Term *src);
int Free_Term(Term *t);

int Construct_Data_Structure_1(Data_Structure *d, int factor, Data_Structure *output);
int Construct_Data_Structure_Independent(Data_Structure *d, int factor, XOR_AND_Expression *alfa,
                                         Data_Structure *output);
int Construct_Data_Structure_Dependent(Data_Structure *d, int factor, XOR_AND_Expression *alfa,
                                       Vector *x, Data_Structure *output);

unsigned int Count(XOR_AND_Expression *e, Data_Structure *d);
int Direct_Sum_1(XOR_AND_Expression *e, int n);

void Print_Expression(XOR_AND_Expression *e);

unsigned int Evaluate_Expression(XOR_AND_Expression *e, int n);




double Interval(unsigned int aa, unsigned int bb, unsigned int *S, unsigned int s0)
{
  double sum = 0;
  unsigned int pw = 0;

  int i, j, k;
  int t_no, bit;

  int var_arr[32];

  int alpha = aa;
  int beta = bb;


  XOR_AND_Expression *a = new XOR_AND_Expression;
  XOR_AND_Expression *a_aux = new XOR_AND_Expression;

  a->exp = new XOR_AND;
  a->exp->term = new Term[529];

  a_aux->exp = new XOR_AND;
  a_aux->exp->term = new Term[529];


  for (j = 0; j < 32; j++)
    var_arr[j] = 0;


  t_no = 0;

  for (j = 0; j < 32; j++)
  {
    for (k = j; k < 32; k++)
    {
      bit = (S[j] >> (31 - k)) & 1;

      if (bit == 1)
      {
        if (k == j)
        {
          var_arr[k] = 1;

          a->exp->term[t_no].n = 1;
          a->exp->term[t_no].fact = new int[1];
          a->exp->term[t_no].fact[0] = k;
        }

        else
        {
          var_arr[k] = var_arr[j] = 1;

          a->exp->term[t_no].n = 2;
          a->exp->term[t_no].fact = new int[2];
          a->exp->term[t_no].fact[0] = j;
          a->exp->term[t_no].fact[1] = k;
        }

        t_no += 1;
      }
    }
  }

  if (s0 == 1)
  {
    a->exp->term[t_no].n = 1;
    a->exp->term[t_no].fact = new int[1];
    a->exp->term[t_no].fact[0] = -1;

    t_no += 1;
  }
  
  a->exp->n = t_no;

  a->var_no = 0;
  for (j = 0; j < 32; j++)
    if (var_arr[j] == 1)
      a->var_no += 1;
      
  
    
  while (alpha <= beta)
  {
    if (((alpha >> pw) & 1U) == 1U)
    {
      a_aux->var_no = a->var_no;
      a_aux->exp->n = a->exp->n;

      for (j = 0; j < a->exp->n; j++)
        Copy_Term(&a_aux->exp->term[j], &a->exp->term[j]);
     
      Simplify_Expression(a_aux, alpha, pw);
      
      if (a_aux->var_no < 32)
      {
        int map_t[32];
        for (j = 0; j < 32; j++)
          map_t[j] = -1;
  
        int v_n = 0;

        for (j = 0; j < a_aux->exp->n; j++)
        {
          Term t = a_aux->exp->term[j];
          if (!((t.n == 1) && (t.fact[0] == -1)))
          {
            for (k = 0; k < t.n; k++)
            {
              if (map_t[t.fact[k]] == -1)
              {
                map_t[t.fact[k]] = v_n;
                v_n += 1;
              }

              a_aux->exp->term[j].fact[k] = map_t[t.fact[k]];
            }
          }
        }
      }
      
      //threshold for applying algorithm or linear summation      
      if (pw < 15)
      {
        sum += Direct_Sum_1(a_aux, pw);
      }
      else
      {
        
        Data_Structure *d = new Data_Structure;
        
        int res = XOR_AND_2_EXACT_COUNT(a_aux, d);
        if (res == 0)
        {
          int result = Count(a_aux, d);
          Free_Data_Structure(d);

          sum = sum + (((result << 1) - (1 << a_aux->var_no)) << (pw - a_aux->var_no));
        }

        delete d;
      }

      for (j = 0; j < a_aux->exp->n; j++)
        Free_Term(&a_aux->exp->term[j]);
      
            
      alpha += (1 << pw);
    }

    if (alpha > beta)
    {
      for (i = 0; i < a->exp->n; i++)
        Free_Term(&a->exp->term[i]);
    
      delete [] a->exp->term;
      delete [] a_aux->exp->term;
  
      delete a->exp;
      delete a_aux->exp;
  
      delete a;
      delete a_aux;
      
      return sum;
    }

    if (((beta >> pw) & 1) == 0)
    {
      a_aux->var_no = a->var_no;
      a_aux->exp->n = a->exp->n;

      for (j = 0; j < a->exp->n; j++)
        Copy_Term(&a_aux->exp->term[j], &a->exp->term[j]);

      Simplify_Expression(a_aux, beta - (1 << pw) + 1, pw);

      if (a_aux->var_no < 32)
      {
        int map_t[32];
        for (j = 0; j < 32; j++)
          map_t[j] = -1;
  
        int v_n = 0;

        for (j = 0; j < a_aux->exp->n; j++)
        {
          Term t = a_aux->exp->term[j];
          if (!((t.n == 1) && (t.fact[0] == -1)))
          {
            for (k = 0; k < t.n; k++)
            {
              if (map_t[t.fact[k]] == -1)
              {
                map_t[t.fact[k]] = v_n;
                v_n += 1;
              }

              a_aux->exp->term[j].fact[k] = map_t[t.fact[k]];
            }
          }
        }
      }
      
      //threshold for applying algorithm or linear summation
      if (pw < 15)
      {
        sum += Direct_Sum_1(a_aux, pw);
      }
      else
      {
        
        Data_Structure *d = new Data_Structure;
        
        int res = XOR_AND_2_EXACT_COUNT(a_aux, d);
        if (res == 0)
        {
          int result = Count(a_aux, d);
          Free_Data_Structure(d);

          sum = sum + (((result << 1) - (1 << a_aux->var_no)) << (pw - a_aux->var_no));
        }

        delete d;
      }
      
      
      for (j = 0; j < a_aux->exp->n; j++)
        Free_Term(&a_aux->exp->term[j]);

            
      if (beta - (1 << pw) > beta)
        beta = 0;
      else
        beta -= (1 << pw);
    }

    pw += 1;
  }

  for (i = 0; i < a->exp->n; i++)
    Free_Term(&a->exp->term[i]);
    
  delete [] a->exp->term;
  delete [] a_aux->exp->term;
  
  delete a->exp;
  delete a_aux->exp;
  
  delete a;
  delete a_aux;
  
  
  return sum;
}


/*
Function simplifies input expression with the first (32-p)-fixed positions of base
Return value:
  0 - OK
 -1 - error
*/
int Simplify_Expression(XOR_AND_Expression *output, unsigned int base, int p)
{
  int i, j;
  int bit, nc, one_no;

  for (i = p; i < 32; i++)
  {
    bit = (base >> i) & 1;
    nc = 0;
    one_no = 0;

    for (j = 0; j < output->exp->n; j++)
    {
      Term t = output->exp->term[j];

      if ((t.n == 1) && (t.fact[0] == -1))
      {
        one_no += 1;
        Free_Term(&output->exp->term[j]);

        continue;
      }

      if ((t.n == 1) && (t.fact[0] == i))
      {
        if (bit == 1)
          one_no += 1;

        Free_Term(&output->exp->term[j]);

        continue;
      }

      if ((t.n == 2) && ((t.fact[0] == i) || (t.fact[1] == i)))
      {
        if (bit == 0)
        {
          Free_Term(&output->exp->term[j]);
          continue;
        }

        if (nc < j)
        {        
	  int t_aux;
	  if (t.fact[0] == i)
	    t_aux = t.fact[1];
	  else
	    t_aux = t.fact[0];
	    
	  int res_simp = Simplify_Term(output, nc, t_aux);
	  if (res_simp == 0)
	  {
            output->exp->term[nc].n = 1;
            output->exp->term[nc].fact = new int[1];
        
            if (t.fact[0] == i)
              output->exp->term[nc].fact[0] = t.fact[1];
            else
              output->exp->term[nc].fact[0] = t.fact[0];
          
            nc += 1;
	  }
	  else if (res_simp == 1)
	  {
	    nc -= 1;
	  }
	  
          Free_Term(&output->exp->term[j]);
          continue;
        }

        else if (nc == j)
        {
	  int t_aux;
	  if (t.fact[0] == i)
	    t_aux = t.fact[1];
	  else
	    t_aux = t.fact[0];
	    
	  int res_simp = Simplify_Term(output, nc, t_aux);
	  if (res_simp == 0)
	  {
            output->exp->term[nc].n = 1;
        
            if (t.fact[0] == i)
              output->exp->term[nc].fact[0] = t.fact[1];

            nc += 1;
	  }
	  else if (res_simp == 1)
	  {
	    nc -= 1;
	    Free_Term(&output->exp->term[j]);
	  }
	  
          continue;
        }
      }

      if (nc < j)
      {
        if (t.n == 1)
	{
	  int res_simp = Simplify_Term(output, nc, t.fact[0]);
	  if (res_simp == 1)
	  {
	    nc -= 1;
            Free_Term(&output->exp->term[j]);
	    
	    continue;
	  }
	}
	
        Copy_Term(&output->exp->term[nc], &output->exp->term[j]);
        Free_Term(&output->exp->term[j]);
	
	nc += 1;
	
	continue;
      }

      if (t.n == 1)
      {
	int res_simp = Simplify_Term(output, nc, t.fact[0]);
	if (res_simp == 1)
	{
	  nc -= 1;
          Free_Term(&output->exp->term[j]);
	  continue;
	}
      }
      
      
      nc += 1;
    }

    if (one_no % 2 == 1)
    {
      output->exp->term[nc].n = 1;
      output->exp->term[nc].fact = new int[1];
      output->exp->term[nc].fact[0] = -1;

      nc += 1;
    }

    output->exp->n = nc;
  }

  int var_arr[32];
  for (i = 0; i < 32; i++)
    var_arr[i] = 0;

  for (j = 0; j < output->exp->n; j++)
  {
    Term t = output->exp->term[j];

    if ((t.n == 1) && (t.fact[0] == -1))
      continue;

    if (t.n == 1)
      var_arr[t.fact[0]] = 1;
    else
      var_arr[t.fact[0]] = var_arr[t.fact[1]] = 1;
  }

  output->var_no = 0;
  for (i = 0; i < 32; i++)
    if (var_arr[i] == 1)
      output->var_no += 1;
    
  return 0;
}


/*
Function eliminates duplicate terms from output expression
Return value:
  0 - no simplification made
  1 - simplification done
*/
int Simplify_Term(XOR_AND_Expression *output, int n, int t_val)
{
  int j;
  
  for (j = 0; j < n; j++)
  {
    Term t = output->exp->term[j];
    
    if ((t.n == 1) && (t.fact[0] == t_val))
    {
      int i;
      for (i = j+1; i < n; i++)
      {
        Free_Term(&output->exp->term[i-1]);
	Copy_Term(&output->exp->term[i-1], &output->exp->term[i]);
      }
      
      Free_Term(&output->exp->term[n-1]);
      
      return 1;
    }
  }
  
  return 0;
}




/*
Function computes the number of combinations satisfaying the expression
Return value:
  no of combinations satisfaying expression
*/
unsigned int Count(XOR_AND_Expression *e, Data_Structure *d)
{
  int n = e->var_no;
  int m = d->t->r;

  unsigned int ret = 1 << (n - m);

  if (d->type == 1)
  {
    ret *= (1 << (m - 1));
  }
  else if (d->type == 2)
  {
    ret *= ((1 << (m - 1)) - (1 << ((m - 2) >> 1)));
  }

  if (d->z == 1)
    ret = (1 << n) - ret;
  
  return ret;
}


/*
Function implements the algorithm for 2-XOR-AND exact counting
Return values:
  0 - success, output data structure contains the result
  1 - error
*/
int XOR_AND_2_EXACT_COUNT(XOR_AND_Expression *input, Data_Structure *output)
{
  int res = 0;
  int i, j;

  
  //BASE CASE
  res = Base_Expression(input, output);
  if (res == 0)
  {
    //input expression is a basic one - end of reccursive calls - going up
    return res;
  }

  //RECCURSIVE CASE
  if (res == 1)
  {
    //expression is not basic - factorize and go in reccursively
    int factor;
    XOR_AND_Expression *alfa, *beta;

    alfa = new XOR_AND_Expression;
    beta = new XOR_AND_Expression;

    res = Factorize(input, factor, alfa, beta);
    if (res != 0)
      return 1;

    //go in reccursively in beta
    Data_Structure *d = new Data_Structure;
    
    res = XOR_AND_2_EXACT_COUNT(beta, d);

    if (res != 0)
    {
      Free_Expression(alfa);
      Free_Expression(beta);

      delete alfa;
      delete beta;

      delete d;

      return 1;
    }


    //algorithm implementation as described in paper
    //construct the output from data structure d
    res = Expression_Is_One(alfa);
    if (res == 1)
    {
      Construct_Data_Structure_1(d, factor, output);

      Free_Expression(alfa);
      Free_Expression(beta);

      Free_Data_Structure(d);

      delete alfa;
      delete beta;

      delete d;
      
      return 0;
    }


    Vector  *x = new Vector;
    
    res = Linear_Independence(d, alfa, x);
    if (res == 0)
    {
      //linear independent
      Construct_Data_Structure_Independent(d, factor, alfa, output);

      Free_Expression(alfa);
      Free_Expression(beta);

      Free_Data_Structure(d);

      delete alfa;
      delete beta;

      delete d;

      delete x;

      return 0;
    }

    else
    {
      //linear dependent - x contains the coefficients
      Construct_Data_Structure_Dependent(d, factor, alfa, x, output);

      delete [] x->elem;
      
      Free_Expression(alfa);
      Free_Expression(beta);

      Free_Data_Structure(d);

      delete alfa;
      delete beta;

      delete d;

      delete x;

      return 0;
    }

  }
    
  return res;
}



/*
Function verifies if the given expression is a basic one
That means that all the variables appear just once in it
Return values:
  -1 - error
   1 - expression is not basic
   0 - expression is basic; data will contain its decomposition
*/
int Base_Expression(XOR_AND_Expression *expr, Data_Structure *data)
{
  int i, j;
  int m = 0;
  int q = 0;
  int n = expr->var_no;

  if (expr->exp->n == 0)
  {
    //expression is empty
    data->z = 0;
    data->type = 2;
    data->unique_elem = -1;
    data->beta_length = 0;

    data->t = new Matrix;
    data->t->r = 0;
    data->t->c = n;
    data->t->elem = NULL;

    data->c = new Vector;
    data->c->n = 0;
    data->c->elem = NULL;

    data->beta_form = NULL;

    return 0;
  }

  int unique = 0;
  int unique_elem = -1;

  int *aux_t = new int[n + 1];
  for (i = 0; i < n + 1; i++)
    aux_t[i] = 0;

  int ad = 1;
  for (i = 0; i < expr->exp->n; i++)
  {
    Term t = expr->exp->term[i];
    if ((t.n == 1) && (t.fact[0] != -1))
    {
      unique_elem = t.fact[0];

      unique += 1;
      if (unique > 1)
      {
        ad = 0;
        break;
      }
    }

    int qq = 0;
    for (j = 0; j < t.n; j++)
    {
      if (t.fact[j] == -1)
      {
        aux_t[n] += 1;
        m -= 1;
        continue;
      }
      else if (aux_t[t.fact[j]] == 1)
      {
        ad = 0;
        break;
      }

      aux_t[t.fact[j]] = 1;

      if (t.fact[j] != -1)
        qq = 1;
    }

    if (qq == 1)
      q += 1;

    m += t.n;

    if (ad == 0)
      break;
  }

  if (ad == 0)
  {
    delete [] aux_t;
    return 1;
  }

  data->type = (unique == 1) ? 1 : 2;
  data->z = aux_t[n] % 2;

  data->c = new Vector;
  data->c->n = (m > 0) ? m : 0;
  if (data->c->n > 0)
  {
    data->c->elem = new int[data->c->n];
    for (i = 0; i < data->c->n; i++)
      data->c->elem[i] = 0;
  }

  data->t = new Matrix;
  data->t->r = (m > 0) ? m : 0;
  data->t->c = n;
  if (data->t->r > 0)
  {
    data->t->elem = new int*[data->t->r];
    for (i = 0; i < data->t->r; i++)
    {
      data->t->elem[i] = new int[data->t->c];
      for (j = 0; j < data->t->c; j++)
        data->t->elem[i][j] = 0;
    }
  }

  data->beta_length = q;

  if (q > 0)
  {
    data->beta_form = new int*[q];
    for (i = 0; i < q; i++)
      data->beta_form[i] = new int[2];
  }

  q = 0;
  j = 0;

  data->unique_elem = -1;
  for (i = 0; i < expr->exp->n; i++)
  {
    Term t = expr->exp->term[i];
    if ((t.n == 1) && (t.fact[0] != -1))
    {
      data->beta_form[q][0] = j;
      data->beta_form[q][1] = -1;

      data->unique_elem = q;
      q += 1;

      data->t->elem[j][t.fact[0]] = 1;
      j += 1;

      continue;
    }

    if ((t.n == 1) && (t.fact[0] == -1))
    {
      continue;
    }

    data->beta_form[q][0] = j;
    data->beta_form[q][1] = j + 1;
    q += 1;

    data->t->elem[j][t.fact[0]] = 1;
    data->t->elem[j + 1][t.fact[1]] = 1;
    j += 2;
  }


  delete [] aux_t;

  return 0;
}



/*
Function searches a factorizer in expression
Return values:
  factor found
  -1 - no factor found
*/
int Find_Factor(XOR_AND_Expression *expr)
{
  int i, j;
  int factor = -1;

  for (i = 0; i < expr->exp->n; i++)
  {
    Term t = expr->exp->term[i];
    for (j = 0; j < t.n; j++)
    {
      if (t.fact[j] != -1)
      {
        factor = t.fact[j];
        return factor;
      }
    }
  }

  return factor;
}



/*
Function copies the content of term src into term dest
Return values:
  0 - always zero
*/
int Copy_Term(Term *dest, Term *src)
{
  int i;

  dest->n = src->n;

  dest->fact = new int[dest->n];
  for (i = 0; i < src->n; i++)
    dest->fact[i] = src->fact[i];

  return 0;
}



/*
Function frees the content of term t
Return values:
  0 - always zero
*/
int Free_Term(Term *t)
{
  t->n = 0;
  delete [] t->fact;

  return 0;
}




/*
Function factorizes term t on factor fact
It extracts fact from t if it is there
Return values:
  -1 - error
   0 - factorized t is returned in out
   1 - fact doesn't appear in t
*/
int Factorize_Term(Term *t, int fact, Term *out)
{
  int i, j;

  int ad = 0;
  for (i = 0; i < t->n; i++)
  {
    if (t->fact[i] == fact)
    {
      ad = 1;
      break;
    }
  }

  if (ad == 0)
    return 1;

  out->n = t->n - 1;
  if (out->n == 0)
  {
    out->n = 1;
    out->fact = new int[1];
    out->fact[0] = -1;

    return 0;
  }

  out->fact = new int[out->n];
  j = 0;
  for (i = 0; i < t->n; i++)
  {
    if (t->fact[i] != fact)
    {
      out->fact[j] = t->fact[i];
      j += 1;
    }
  }

  return 0;
}



/*
Function factorizes expression expr on factor fact
It applies Factorize_Term to every term in expression expr
  - alfa contains the factorized terms
  - beta contains the unfactorized ones
Return values:
   0 - always zero
*/
int Factorize_Expression(XOR_AND_Expression *expr, int fact, XOR_AND_Expression *alfa, XOR_AND_Expression *beta)
{
  int i, j;
  int k_alfa, k_beta;

  int n = expr->exp->n;
  Term *ts = new Term[n];

  j = 0;
  for (i = 0; i < n; i++)
  {
    int res = Factorize_Term(&expr->exp->term[i], fact, &ts[i]);
    if (res == 1)
    {
      //term was not factorized - goes in beta
      ts[i].n = -1;
      j += 1;
    }
  }

  alfa->var_no = beta->var_no = expr->var_no;

  alfa->exp = new XOR_AND;
  beta->exp = new XOR_AND;
  
  alfa->exp->n = n - j;
  beta->exp->n = j;

  if (n - j > 0)
    alfa->exp->term = new Term[n - j];

  if (j > 0)
    beta->exp->term = new Term[j];

  k_alfa = k_beta = 0;
  for (i = 0; i < n; i++)
  {
    if (ts[i].n == -1)
    {
      //goes in beta
      Copy_Term(&beta->exp->term[k_beta], &expr->exp->term[i]);
      k_beta += 1;
    }
    else
    {
      //goes in alfa
      Copy_Term(&alfa->exp->term[k_alfa], &ts[i]);
      k_alfa += 1;
    }
  }

  for (i = 0; i < n; i++)
  {
    if (ts[i].n != -1)
      delete [] ts[i].fact;
  }

  delete [] ts;

  return 0;
}



/*
Function factorizes expression expr into alfa and beta
It finds the factor to factorize on - factor
Return values:
  0 - factorization OK
  1 - no possible factorization
*/
int Factorize(XOR_AND_Expression *expr, int &factor, XOR_AND_Expression *alfa, XOR_AND_Expression *beta)
{
  factor = Find_Factor(expr);
  if (factor == -1)
    return 1;

  Factorize_Expression(expr, factor, alfa, beta);
  return 0;
}



/*
Function frees space used by an expression
*/
int Free_Expression(XOR_AND_Expression *e)
{
  int i;
  int n = e->exp->n;

  for (i = 0; i < n; i++)
  {
    Term t = e->exp->term[i];
    delete [] t.fact;
  }

  if (n > 0)
    delete [] e->exp->term;

  delete e->exp;

  return 0;
}


/*
Function frees space used by a data structure
*/
int Free_Data_Structure(Data_Structure *d)
{
  int i;

  if (d->c->n > 0)
  {
    delete [] d->c->elem;
    d->c->n = 0;
  }
  delete d->c;

  if (d->t->r > 0)
  {
    for (i = 0; i < d->t->r; i++)
      delete [] d->t->elem[i];

    delete [] d->t->elem;
    d->t->r = 0;
  }
  delete d->t;

  if (d->beta_length > 0)
  {
    for (i = 0; i < d->beta_length; i++)
      delete [] d->beta_form[i];

    delete [] d->beta_form;
    d->beta_length = 0;
  }

  return 0;
}



/*
Function implements the Gaussian-Elimination Algorithm
for systems of linear equations
The number of equations can be equal or greater than the number of unknowns
A squared system is solved and the extra equations are than checked
Return values:
  0 - system has not a valid solution
  1 - system has a solution that is contained by the vector x
*/
int Gauss_Elimination(int n, int m, Matrix *A, Vector *b, Vector *x)
{
  int i, j, k;
  int index;
  int temp;

  x->n = m;
  x->elem = new int[m];

  for (i = 0; i < m; i++)
  {
    //find an equation that has 1 on position i
    index = -1;
    
    for (j = i; j < n; j++)
      if (A->elem[j][i] == 1)
      {
        index = j;
        break;
      }

    if (index == -1)
      continue;
      
    if (index != i)
    {
      //swap equations index and i
      for (j = 0; j < m; j++)
      {
        temp = A->elem[i][j];
        A->elem[i][j] = A->elem[index][j];
        A->elem[index][j] = temp;
      }

      temp = b->elem[i];
      b->elem[i] = b->elem[index];
      b->elem[index] = temp;
    }


    //make 0 all the elements on column i
    for (j = i + 1; j < n; j++)
    {
      if (A->elem[j][i] == 1)
      {
        for (k = i; k < m; k++)
          A->elem[j][k] ^= A->elem[i][k];

        b->elem[j] ^= b->elem[i];
      }
    }
  }


  //construct the solution from back to front
  index = -1;
  for (i = m - 1; i >= 0; i--)
  {
    temp = 0;
    for (j = i + 1; j < m; j++)
      temp += A->elem[i][j] * x->elem[j];
    
    temp = temp % 2;

    if (A->elem[i][i] == 1)
    {
      x->elem[i] = temp ^ b->elem[i];
    }
    else
    {
      if (temp == b->elem[i])
        x->elem[i] = 0;
      else
      {
        index = 1;
        break;
      }
    }
  }

  if (index != -1)
  {
    //system has no solution - linear independent
    delete [] x->elem;
    return 0;
  }


  //verify the eventual extra equations for correctness
  index = -1;
  for (i = m; i < n; i++)
  {
    temp = 0;
    for (j = 0; j < m; j++)
      temp += A->elem[i][j] * x->elem[j];

    temp %= 2;
    if (temp != b->elem[i])
    {
      //problems with this equation
      index = i;
      break;
    }
  }

  if (index != -1)
  {
    //system has no solution - linear independent
    delete [] x->elem;
    return 0;
  }

  return 1;
}



/*
Function decomposes vector alfa as a linear combination of vectors in beta, if possible
a will contain the coefficients corresponding to these vectors
Return values:
  0 - alfa is linear independent of beta
  1 - alfa is linear dependent of beta; coef contains the coefficients
*/
int Linear_Independence(Data_Structure *beta, XOR_AND_Expression *alfa, Vector *coef)
{
  int i, j;
  int res = 1;

  int syst_dim = 0;

  if ((beta->beta_length == 0) || (alfa->exp->n == 0))
    return 0;

  //transform expression alfa into a vector of d dimensions
  //alfa has a special form - is factorized on one of the dimensions
  Vector *alfa_v, *b;
  alfa_v = new Vector;
  b = new Vector;

  alfa_v->n = alfa->var_no;
  alfa_v->elem = new int[alfa->var_no];

  for (i = 0; i < alfa_v->n; i++)
    alfa_v->elem[i] = 0;

  for (i = 0; i < alfa->exp->n; i++)
  {
    Term t = alfa->exp->term[i];
    if (t.fact[0] != -1)
    {
      alfa_v->elem[t.fact[0]] = 1;
    }
  }


  //construct the matrix to use in determining the coefficients
  b->n = alfa_v->n;
  b->elem = new int[alfa_v->n];

  Matrix *a = new Matrix;
  a->r = beta->t->c;
  a->c = beta->t->r;

  a->elem = new int*[a->r];
  for (i = 0; i < a->r; i++)
    a->elem[i] = new int[a->c];

  Vector *t_coord = new Vector;;
  t_coord->n = b->n;
  t_coord->elem = new int[t_coord->n];

  syst_dim = 0;
  for (i = 0; i < a->r; i++)
  {
    int zero_c = 0;
    for (j = 0; j < a->c; j++)
    {
      a->elem[syst_dim][j] = beta->t->elem[j][i];
      if (beta->t->elem[j][i] == 1)
        zero_c += 1;
    }

    if ((zero_c == 0) && (alfa_v->elem[i] == 1))
    {
      res = 0;
      break;
    }

    if (zero_c > 0)
    {
      //eliminate duplicate equations - imply linear independence
      int ii, jj;
      int bb = 1;

      for (ii = 0; ii < syst_dim; ii++)
      {
        int aa = 1;
        for (jj = 0; jj < a->c; jj++)
        {
          if (a->elem[syst_dim][jj] != a->elem[ii][jj])
          {
            aa = 0;
            break;
          }
        }

        if (aa == 1)
        {
          if (alfa_v->elem[i] == b->elem[ii])
            bb = 0;
          else
            res = 0;
        }

      }

      if (res == 0)
        break;

      if (bb == 1)
      {
        b->elem[syst_dim] = alfa_v->elem[i];
        t_coord->elem[i] = syst_dim;

        syst_dim += 1;
      }
    }

    else
    {
      t_coord->elem[i] = -1;
    }
  }


  try
  {
    if (res == 0)
      throw "Linear independent";

    int k = a->c;
    if (syst_dim < k)
    {
      //the number of equations is less than the number of parameters
      //linear independent
      throw "Linear independent";
    }

    res = Gauss_Elimination(syst_dim, k, a, b, coef);
    if (res == 0)
      throw "Linear independent";
  }

  catch(const char *independent)
  {
    //linear independent
    for (i = 0; i < a->r; i++)
      delete [] a->elem[i];
    delete [] a->elem;

    delete [] alfa_v->elem;
    delete [] b->elem;
    delete [] t_coord->elem;

    delete a;
    delete alfa_v;
    delete b;
    delete t_coord;

    return 0;
  }


  for (i = 0; i < a->r; i++)
    delete [] a->elem[i];
  delete [] a->elem;

  delete [] alfa_v->elem;
  delete [] b->elem;
  delete [] t_coord->elem;

  delete a;
  delete alfa_v;
  delete b;
  delete t_coord;

  return 1;
}



/*
Function checks if given expression is the constant 1
Return values:
  1 - true
  0 - false
*/
int Expression_Is_One(XOR_AND_Expression *expr)
{
  int n = expr->exp->n;
  if (n != 1)
    return 0;

  Term t = expr->exp->term[0];
  if (t.n != 1)
    return 0;

  if (t.fact[0] != -1)
    return 0;

  return 1;
}



/*
Function computes the new data structure from the one that returns from recursion
for the case when alfa is equal with 1
*/
int Construct_Data_Structure_1(Data_Structure *d, int factor, Data_Structure *output)
{
  int i, j;

  output->z = d->z;
  output->type = 1;

  output->t = new Matrix;
  output->c = new Vector;

  if (d->type == 1)
  {
    output->beta_length = d->beta_length;
    output->unique_elem = d->unique_elem;

    if (output->beta_length > 0)
    {
      output->beta_form = new int*[output->beta_length];
      for (i = 0; i < output->beta_length; i++)
      {
        output->beta_form[i] = new int[2];

        output->beta_form[i][0] = d->beta_form[i][0];
        output->beta_form[i][1] = d->beta_form[i][1];
      }
    }
    
    output->t->r = d->t->r;
    output->t->c = d->t->c;

    if (d->t->r > 0)
    {
      output->t->elem = new int*[d->t->r];

      output->c->n = d->c->n;
      output->c->elem = new int[d->c->n];

      for (i = 0; i < d->t->r; i++)
      {
        output->t->elem[i] = new int[d->t->c];
        for (j = 0; j < d->t->c; j++)
          output->t->elem[i][j] = d->t->elem[i][j];

        output->c->elem[i] = d->c->elem[i];
      }

      output->t->elem[d->beta_form[d->unique_elem][0]][factor] = 1;
    }
  }

  else if (d->type == 2)
  {
    output->beta_length = d->beta_length + 1;
    output->unique_elem = output->beta_length - 1;

    if (output->beta_length > 0)
    {
      output->beta_form = new int*[output->beta_length];

      output->beta_form[output->beta_length - 1] = new int[2];
      output->beta_form[output->beta_length - 1][0] = d->t->r;
      output->beta_form[output->beta_length - 1][1] = -1;

      for (i = 0; i < output->beta_length - 1; i++)
      {
        output->beta_form[i] = new int[2];

        output->beta_form[i][0] = d->beta_form[i][0];
        output->beta_form[i][1] = d->beta_form[i][1];
      }
    }

    output->t->r = d->t->r + 1;
    output->t->c = d->t->c;

    if (d->t->r + 1 > 0)
    {
      output->t->elem = new int*[d->t->r + 1];

      output->c->n = d->c->n + 1;
      output->c->elem = new int[d->c->n + 1];

      for (i = 0; i < d->t->r; i++)
      {
        output->t->elem[i] = new int[d->t->c];
        for (j = 0; j < d->t->c; j++)
          output->t->elem[i][j] = d->t->elem[i][j];

        output->c->elem[i] = d->c->elem[i];
      }

      output->t->elem[d->t->r] = new int[d->t->c];
      for (j = 0; j < d->t->c; j++)
        output->t->elem[d->t->r][j] = 0;

      output->c->elem[d->c->n] = 0;

      output->t->elem[d->t->r][factor] = 1;
    }

  }

  return 0;
}



/*
Function computes the new data structure from the one that returns from recursion
for the case when alfa is linear independent of the structure beta
*/
int Construct_Data_Structure_Independent(Data_Structure *d, int factor, XOR_AND_Expression *alfa,
                                         Data_Structure *output)
{
  int i, j;

  output->z = d->z;
  output->type = d->type;

  output->t = new Matrix;
  output->c = new Vector;

  output->beta_length = d->beta_length + 1;
  output->unique_elem = d->unique_elem;

  output->beta_form = new int*[output->beta_length];
  for (i = 0; i < output->beta_length - 1; i++)
  {
    output->beta_form[i] = new int[2];

    output->beta_form[i][0] = d->beta_form[i][0];
    output->beta_form[i][1] = d->beta_form[i][1];
  }

  output->beta_form[output->beta_length - 1] = new int[2];
  output->beta_form[output->beta_length - 1][0] = d->t->r;
  output->beta_form[output->beta_length - 1][1] = d->t->r + 1;


  output->t->r = d->t->r + 2;
  output->t->c = d->t->c;
  output->t->elem = new int*[d->t->r + 2];

  output->c->n = d->c->n + 2;
  output->c->elem = new int[d->c->n + 2];

  for (i = 0; i < d->t->r; i++)
  {
    output->t->elem[i] = new int[d->t->c];
    for (j = 0; j < d->t->c; j++)
      output->t->elem[i][j] = d->t->elem[i][j];

    output->c->elem[i] = d->c->elem[i];
  }

  output->t->elem[d->t->r] = new int[d->t->c];
  output->t->elem[d->t->r + 1] = new int[d->t->c];
  for (j = 0; j < d->t->c; j++)
  {
    output->t->elem[d->t->r][j] = 0;
    output->t->elem[d->t->r + 1][j] = 0;
  }

  output->c->elem[d->c->n] = 0;
  output->c->elem[d->c->n + 1] = 0;

  output->t->elem[d->t->r][factor] = 1;

  for (i = 0; i < alfa->exp->n; i++)
  {
    Term t = alfa->exp->term[i];

    if (t.fact[0] == -1)
      output->c->elem[d->c->n + 1] = 1;
    else
      output->t->elem[d->t->r + 1][t.fact[0]] = 1;
  }

  return 0;
}



/*
Function computes the new data structure from the one that returns from recursion
for the case when alfa is linear dependent of the structure beta
*/
int Construct_Data_Structure_Dependent(Data_Structure *d, int factor, XOR_AND_Expression *alfa,
                                       Vector *x, Data_Structure *output)
{
  int i, j;

  int alfa_z = 0;
  for (i = 0; i < alfa->exp->n; i++)
  {
    Term t = alfa->exp->term[i];

    if (t.fact[0] == -1)
    {
      alfa_z = 1;
      break;
    }
  }


  output->t = new Matrix;
  output->c = new Vector;


  //eliminating all the double terms
  for (i = 0; i < d->beta_length; i++)
  {
    if (((d->type == 1) && (d->unique_elem != i)) || (d->type == 2))
    {
      if ((x->elem[d->beta_form[i][0]] == 1) && (x->elem[d->beta_form[i][1]] == 1))
      {
        //case 3.a
        d->t->elem[d->beta_form[i][0]][factor] = 1;
        d->t->elem[d->beta_form[i][1]][factor] = 1;

        if (((d->c->elem[d->beta_form[i][0]] == 1) && (d->c->elem[d->beta_form[i][1]] == 1)) ||
            ((d->c->elem[d->beta_form[i][0]] == 0) && (d->c->elem[d->beta_form[i][1]] == 0)))
        {
          alfa_z ^= 1;
        }

        continue;
      }

      if (x->elem[d->beta_form[i][0]] == 1)
      {
        //case 3.b
        d->t->elem[d->beta_form[i][1]][factor] = 1;

        if (d->c->elem[d->beta_form[i][0]] == 1)
        {
          alfa_z ^= 1;
        }

        continue;
      }

      if (x->elem[d->beta_form[i][1]] == 1)
      {
        //case 3.b
        d->t->elem[d->beta_form[i][0]][factor] = 1;

        if (d->c->elem[d->beta_form[i][1]] == 1)
        {
          alfa_z ^= 1;
        }
      }
    }
  }


  if (d->type == 2)
  {
    if (alfa_z == 1)
    {
      //case 3.d
      output->type = 1;
      output->z = d->z;

      output->beta_length = d->beta_length + 1;
      output->unique_elem = output->beta_length - 1;

      output->beta_form = new int*[output->beta_length];

      output->beta_form[output->beta_length - 1] = new int[2];
      output->beta_form[output->beta_length - 1][0] = d->t->r;
      output->beta_form[output->beta_length - 1][1] = -1;

      for (i = 0; i < output->beta_length - 1; i++)
      {
        output->beta_form[i] = new int[2];

        output->beta_form[i][0] = d->beta_form[i][0];
        output->beta_form[i][1] = d->beta_form[i][1];
      }

      output->t->r = d->t->r + 1;
      output->t->c = d->t->c;
      output->t->elem = new int*[d->t->r + 1];

      output->c->n = d->c->n + 1;
      output->c->elem = new int[d->c->n + 1];

      for (i = 0; i < d->t->r; i++)
      {
        output->t->elem[i] = new int[d->t->c];
        for (j = 0; j < d->t->c; j++)
          output->t->elem[i][j] = d->t->elem[i][j];

        output->c->elem[i] = d->c->elem[i];
      }

      output->t->elem[d->t->r] = new int[d->t->c];
      for (j = 0; j < d->t->c; j++)
        output->t->elem[d->t->r][j] = 0;

      output->c->elem[d->c->n] = 0;

      output->t->elem[d->t->r][factor] = 1;
    }

    else if (alfa_z == 0)
    {
      output->type = 2;
      output->z = d->z;

      output->beta_length = d->beta_length;
      output->unique_elem = -1;

      output->beta_form = new int*[output->beta_length];
      for (i = 0; i < output->beta_length; i++)
      {
        output->beta_form[i] = new int[2];

        output->beta_form[i][0] = d->beta_form[i][0];
        output->beta_form[i][1] = d->beta_form[i][1];
      }

      output->t->r = d->t->r;
      output->t->c = d->t->c;
      output->t->elem = new int*[d->t->r];

      output->c->n = d->c->n;
      output->c->elem = new int[d->c->n];

      for (i = 0; i < d->t->r; i++)
      {
        output->t->elem[i] = new int[d->t->c];
        for (j = 0; j < d->t->c; j++)
          output->t->elem[i][j] = d->t->elem[i][j];

        output->c->elem[i] = d->c->elem[i];
      }
    }
  }

  else if (d->type == 1)
  {
    //see if alfa contains y0 in its decomposition
    int d_alfa = (x->elem[d->beta_form[d->unique_elem][0]] == 1) ? 1 : 0;

    if ((d_alfa == 1) && (alfa_z == 1))
    {
      output->type = 2;
      output->z = d->z ^ 1;

      output->beta_length = d->beta_length;
      output->unique_elem = -1;

      output->beta_form = new int*[output->beta_length];
      for (i = 0; i < output->beta_length; i++)
      {
        output->beta_form[i] = new int[2];

        output->beta_form[i][0] = d->beta_form[i][0];
        output->beta_form[i][1] = d->beta_form[i][1];
      }

      output->t->r = d->t->r + 1;
      output->t->c = d->t->c;
      output->t->elem = new int*[d->t->r + 1];

      output->c->n = d->c->n + 1;
      output->c->elem = new int[d->c->n + 1];

      for (i = 0; i < d->t->r; i++)
      {
        output->t->elem[i] = new int[d->t->c];
        for (j = 0; j < d->t->c; j++)
          output->t->elem[i][j] = d->t->elem[i][j];

        output->c->elem[i] = d->c->elem[i];
      }

      output->t->elem[d->t->r] = new int[d->t->c];
      for (j = 0; j < d->t->c; j++)
        output->t->elem[d->t->r][j] = 0;

      output->c->elem[d->c->n] = 1;
      output->t->elem[d->t->r][factor] = 1;

      output->c->elem[d->beta_form[d->unique_elem][0]] ^= 1;
      output->beta_form[d->unique_elem][1] = d->t->r;
    }

    else if ((d_alfa == 1) && (alfa_z == 0))
    {
      output->type = 2;
      output->z = d->z;

      output->beta_length = d->beta_length;
      output->unique_elem = -1;

      output->beta_form = new int*[output->beta_length];
      for (i = 0; i < output->beta_length; i++)
      {
        output->beta_form[i] = new int[2];

        output->beta_form[i][0] = d->beta_form[i][0];
        output->beta_form[i][1] = d->beta_form[i][1];
      }

      output->t->r = d->t->r + 1;
      output->t->c = d->t->c;
      output->t->elem = new int*[d->t->r + 1];

      output->c->n = d->c->n + 1;
      output->c->elem = new int[d->c->n + 1];

      for (i = 0; i < d->t->r; i++)
      {
        output->t->elem[i] = new int[d->t->c];
        for (j = 0; j < d->t->c; j++)
          output->t->elem[i][j] = d->t->elem[i][j];

        output->c->elem[i] = d->c->elem[i];
      }

      output->t->elem[d->t->r] = new int[d->t->c];
      for (j = 0; j < d->t->c; j++)
        output->t->elem[d->t->r][j] = 0;

      output->c->elem[d->c->n] = 1;
      output->t->elem[d->t->r][factor] = 1;

      output->beta_form[d->unique_elem][1] = d->t->r;
    }

    else if ((d_alfa == 0) && (alfa_z == 1))
    {
      output->type = 1;
      output->z = d->z;

      output->beta_length = d->beta_length;
      output->unique_elem = d->unique_elem;

      output->beta_form = new int*[output->beta_length];
      for (i = 0; i < output->beta_length; i++)
      {
        output->beta_form[i] = new int[2];

        output->beta_form[i][0] = d->beta_form[i][0];
        output->beta_form[i][1] = d->beta_form[i][1];
      }

      output->t->r = d->t->r;
      output->t->c = d->t->c;
      output->t->elem = new int*[d->t->r];

      output->c->n = d->c->n;
      output->c->elem = new int[d->c->n];

      for (i = 0; i < d->t->r; i++)
      {
        output->t->elem[i] = new int[d->t->c];
        for (j = 0; j < d->t->c; j++)
          output->t->elem[i][j] = d->t->elem[i][j];

        output->c->elem[i] = d->c->elem[i];
      }

      output->t->elem[d->beta_form[d->unique_elem][0]][factor] = 1;
    }

    else if ((d_alfa == 0) && (alfa_z == 0))
    {
      output->type = 1;
      output->z = d->z;

      output->beta_length = d->beta_length;
      output->unique_elem = d->unique_elem;

      output->beta_form = new int*[output->beta_length];
      for (i = 0; i < output->beta_length; i++)
      {
        output->beta_form[i] = new int[2];

        output->beta_form[i][0] = d->beta_form[i][0];
        output->beta_form[i][1] = d->beta_form[i][1];
      }

      output->t->r = d->t->r;
      output->t->c = d->t->c;
      output->t->elem = new int*[d->t->r];

      output->c->n = d->c->n;
      output->c->elem = new int[d->c->n];

      for (i = 0; i < d->t->r; i++)
      {
        output->t->elem[i] = new int[d->t->c];
        for (j = 0; j < d->t->c; j++)
          output->t->elem[i][j] = d->t->elem[i][j];

        output->c->elem[i] = d->c->elem[i];
      }
    }
  }


  return 0;
}



/*
Function print the expression sent as parameter
*/
void Print_Expression(XOR_AND_Expression *e)
{
  int i, j;

  for (i = 0; i < e->exp->n; i++)
  {
    Term t = e->exp->term[i];

    printf("(");
    for (j = 0; j < t.n - 1; j++)
    {
      printf("%d ", t.fact[j]);
    }
    printf("%d) ", t.fact[j]);

  }
}



/*
Function used in simulation for computing the value of an expression
Return value:
  no of combinations satisfaying the equation
*/
unsigned int Evaluate_Expression(XOR_AND_Expression *e, int n)
{
  int i, j, k;
  unsigned int v;

  unsigned int n_sol = 0;

  for (v = 0; v < (1 << e->var_no); v++)
  {
    unsigned int res = 0;
    for (i = 0; i < e->exp->n; i++)
    {
      Term t = e->exp->term[i];

      if ((t.n == 1) && (t.fact[0] == -1))
        res ^= 1;

      else
      {
        unsigned int f = 1;
        for (j = 0; j < t.n; j++)
          f &= ((v >> t.fact[j]) & 1);

        res ^= f;
      }
    }

    if (res == 1)
      n_sol += 1;
  }

  n_sol = (n_sol << (n - e->var_no));
  return n_sol;
}



/*
Function computes sum of xis (+1,-1) over a dyadic interval
Return value:
  sum of xis (+1,-1)
*/
int Direct_Sum_1(XOR_AND_Expression *e, int n)
{
  int i, j;
  int sum = 0;
  
  unsigned int v;

  for (v = 0; v < (1 << e->var_no); v++)
  {
    unsigned int res = 0;
    for (i = 0; i < e->exp->n; i++)
    {
      Term t = e->exp->term[i];

      if ((t.n == 1) && (t.fact[0] == -1))
        res ^= 1;

      else
      {
        unsigned int f = 1;
        for (j = 0; j < t.n; j++)
          f &= ((v >> t.fact[j]) & 1);

        res ^= f;
      }
    }

    if (res == 1)
      sum += 1;
    else
      sum -= 1;
  }

  sum = (sum << (n - e->var_no));
  return sum;
}

#endif
