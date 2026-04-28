#ifndef SINCOS_H
#define SINCOS_H

//*************************************************************
// Name: sincos.h
// Author: Larry Pyeatt
// Date: 2/22/2018
//*************************************************************
// This file provides functions for calculating sine and
// cosine using fixed-point arithmetic. It uses the first
// nine terms in the Taylor series.

#define pi 0x3243F6A8    // pi as an S(3,28)
#define pi_2 0x1921FB54	 // pi/2 as an S(3,28)
#define pi_x2 0x6487ED51 // 2*pi as an S(3,28)

//*************************************************************
// fixed_sin_C applies symmetries to reduce the range of
// the input, then calls sinq and adjusts the result.
int fixed_sin_C(int x);

//*************************************************************
// fixed_cos_C applies the sin/cos relation to
// the input, then calls fixed_sin_C
int fixed_cos_C(int x);

#endif
