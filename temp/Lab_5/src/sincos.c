#include <stdio.h>
#include <sincos.h>
//*************************************************************
// Name: sincos.c
// Author: Larry Pyeatt
// Date: 2/22/2018
//*************************************************************
// This file provides functions for calculating sine and
// cosine using fixed-point arithmetic. It uses the first
// nine terms in the Taylor series.

// Define a structure that holds coefficient and shift.
struct tabentry{ 
  int coeff; 
  int shift;
};

// create a table of coefficients and shift amounts.
#define TABSIZE 8
static struct tabentry sintab[]={
  {0xAAAAAAAA,  0},  // term 2
  {0x44444445,  2},  // term 3
  {0x97F97F97,  6},  // term 4
  {0x5C778E96,  10}, // term 5
  {0x9466EA60,  15}, // term 6
  {0x5849184F,  20}, // term 7
  {0x94603063,  26}, // term 8
  {0x654B1DC1,  32}  // term 9
};

//*************************************************************
// sinq is used internally by the sine and cosine functions
//   input: x as an S(1,30) s.t. 0 <= x <= pi/2
//   returns sin(x) as an S(3,28)
static int sinq(int x)
{
  long long sum;
  long long tmp;
  long long curpower = x;
  long long xsq;
  int i=0;
  sum = (long long)x << 31;    // initialize 64-bit sum to x
  xsq = (long long)x*(long long)x; // calculate x^2 as an S(3,60)
  xsq >>= 31;                  // convert x^2 to an S(2,29)
  do
    {
      // calculate x^(2n-1) as an S(3,28)
      curpower = ((curpower * xsq) >> 31);
      // multiply x^(2n-1) by coefficient from the table
      tmp = curpower * sintab[i].coeff;
      if(tmp < 0) // if resulting term in negative
	tmp++;    //  add one to avoid round-off error
      tmp >>= sintab[i].shift; // convert the term to S(2,61)
      sum += tmp; // add it to running total
    }  
  while(++i<TABSIZE);
  return (sum >> 33);  // convert result to S(3,28) and return
}

//*************************************************************
// fixed_sin_C applies symmetries to reduce the range of
// the input, then calls sinq and adjusts the result.
int fixed_sin_C(int x)
{
  while(x<0)
    x += pi_x2;
  while(x>pi_x2)
    x -= pi_x2;
  if(x<=pi_2)
    return sinq(x<<2);
  if(x<=pi)
    return sinq((pi-x)<<2);
  if(x<=(pi+pi_2))
    return -sinq((x-pi)<<2);
  return  -sinq((pi_x2 -x)<<2);
}

//*************************************************************
// fixed_cos_C applies the sin/cos relation to
// the input, then calls fixed_sin_C
int fixed_cos_C(int x)
{
  if(x<=0)
    x += pi_x2;
  x = pi_2 - x;
  return fixed_sin_C(x);
}
