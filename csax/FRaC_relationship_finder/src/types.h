#ifndef TYPES_H
#define TYPES_H

#include <limits>

//Use the float type for floats in this FRaC binary.
//Architecture: Consider the consequences of these epsilon values
//FRACFLOAT_EPSILON should be a small nonzero constant with a finite log value.
#ifdef FRAC_FLOAT_64
typedef double fracfloat_t;
#define FRACFLOAT_EPSILON .0000000000001
#define log2ff log2
#define exp2ff exp2
#else
typedef float fracfloat_t;
#define FRACFLOAT_EPSILON .0000001
#define log2ff log2f
#define exp2ff exp2f
#endif

#define FRACFLOAT_INFINITY (std::numeric_limits<fracfloat_t>::infinity())
//Detail: This works fine with most floating point standards, but isn't necessarily safe.
#define FRACFLOAT_NEGATIVE_INFINITY (-FRACFLOAT_INFINITY)

//Detail: FRACFLOAT_EPSILON should be the smallest value such that log2ff(FRACFLOAT_EPSILON) is finite.

#endif
