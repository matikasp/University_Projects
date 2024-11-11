#ifndef _ARY_H_
#define _ARY_H_

#include "stdbool.h"
#include "math.h"
#include "assert.h"

typedef struct value {
    /*
     * left and right end of the interval
     * in my solution, I did not assume that leftVal must be less than rightVal
     * I assume that simply leftVal is the closure from the left side and rightVal from the right side
     * so for example in the interval (-inf, -3] u [3, +inf) -3 will be rightVal, and 3 leftVal
     * they take NAN if there is no corresponding right or left closure
     */
    union {
        struct {
            double leftVal;
            double rightVal;
        };
        double vals[2];
    };

    /*
     * whether the set is empty or all real numbers
     * if empty or real are true then leftVal and rightVal are NAN
     */
    bool empty, real;
} value;

/* Implicitly we assume that all double arguments are real numbers  */
/* i.e., they are different from HUGE_VAL, -HUGE_VAL, and NAN. */

/* value_precision(x, p) returns a value representing  */
/* x +/- p%                                           */
/* initial condition: p > 0                           */
value value_precision(double x, double p);

/* value_from_to(x, y) returns a value representing [x;y] */
/* initial condition: x <= y                              */
value value_from_to(double x, double y);

/* value_exact(x) returns a value representing [x;x] */
value value_exact(double x);

/* in_value(v, x) = x \in v                             */
bool in_value(value v, double x);

/* min_value(v) = the smallest possible value in v,     */
/* or -HUGE_VAL if there is no lower bound.             */
double min_value(value v);

/* max_value(v) = the largest possible value in v,      */
/* or HUGE_VAL if there is no upper bound.              */
double max_value(value v);

/* the midpoint of the interval from min_value to max_value, */
/* or nan if min and max_value are not defined.              */
double mid_value(value v);

/* Arithmetic operations on imprecise values.          */
value add(value a, value b);
value subtract(value a, value b);
value multiply(value a, value b);
value divide(value a, value b);

#endif