#include "ary.h"
#include <stdio.h>

/* function from moodle checking if a number is zero */
bool eq(double x, double y) { return fabs(x - y) < 1e-10; }
bool le(double x, double y) { return x < y || eq(x, y); } // <=
bool ge(double x, double y) { return x > y || eq(x, y); } // >=

typedef enum range_type_t {
    EMPTY, // empty set
    REALS, // all real numbers
    INF_R, // (-inf, rightVal]
    L_INF, // [leftVal, inf)
    INF_RL_INF, // (-inf, rightVal] u [leftVal, inf)
    LR, // [leftVal, rightVal]
} range_type_t;

range_type_t range_type(value v) {
    if (v.real)
        return REALS;
    if (v.empty)
        return EMPTY;
    if (isnan(v.leftVal))
        return INF_R;
    if (isnan(v.rightVal))
        return L_INF;
    if (v.rightVal < v.leftVal)
        return INF_RL_INF;
    return LR;
}

bool has_inf(value v) {
    return !v.empty && (isnan(v.leftVal) || isnan(v.rightVal) || v.leftVal > v.rightVal);
}

const value EMPTY_RANGE = {
    .empty = true,
    .real = false,
    .leftVal = NAN,
    .rightVal = NAN
};

const value REALS_RANGE = {
    .empty = false,
    .real = true,
    .leftVal = NAN,
    .rightVal = NAN
};

/* function returning the minimum value of an array */
double min_tab(double const tab[], int tabLength) {
    double min = tab[0];
    for (int i = 1; i<tabLength; i++) {
        min = fmin(min, tab[i]);
    }
    return min;
}

/* function returning the maximum value of an array */
double max_tab(double const tab[], int tabLength) {
    double max = tab[0];
    for (int i = 1; i<tabLength; i++) {
        max = fmax(max, tab[i]);
    }
    return max;
}

/* function checking if the value is zero */
bool is_value_zero(value v) {
    return eq(v.leftVal, 0) && eq(v.rightVal, 0);
}

/* function checking if the value contains the neighborhood of zero
 * i.e., if it contains numbers less than and greater than 0 */
bool is_zero_neighborhood(value v) {
    switch (range_type(v)) {
        case EMPTY:
            return false;
        case REALS:
            return true;
        case INF_R:
            return v.rightVal > 0;
        case L_INF:
            return v.leftVal < 0;
        case INF_RL_INF:
            return v.leftVal < 0 || v.rightVal > 0;
        default: // LR
            return v.leftVal < 0 && v.rightVal > 0;
    };
}

bool is_one_sided_zero_neighborhood(value v) {
    return (
        (range_type(v) & has_inf(v)) &&
        (eq(v.leftVal, 0.0) || eq(v.rightVal, 0.0))
    );
}

value value_from_min_to_inf(double min) {
    const value result = {
        .empty = false,
        .real = false,
        .leftVal = min,
        .rightVal = NAN,
    };
    return result;
}

value value_from_inf_to_max(double max) {
    const value result = {
        .empty = false,
        .real = false,
        .leftVal = NAN,
        .rightVal = max,
    };
    return result;
}

value opposite(value v) {
    const value result = {
        .empty = v.empty,
        .real = v.real,
        .leftVal = -v.rightVal,
        .rightVal = -v.leftVal,
    };
    return result;
}

value reciprocal(value v) {
    const range_type_t v_type = range_type(v);
    if ((v_type == INF_R && eq(v.rightVal, 0)) || (v_type == L_INF && eq(v.leftVal, 0)))
        return v;
    if (v_type == L_INF)
        return value_from_to(0.0, 1.0 / v.leftVal);
    if (v_type == INF_R)
        return value_from_to(1.0 / v.rightVal, 0.0);
    if (eq(v.leftVal, 0.0)) // [0, x] or (-inf, x] u [0, +inf), x != 0
        return value_from_min_to_inf(1.0 / v.rightVal);
    if (eq(v.rightVal, 0.0)) // [x, 0] or (-inf, 0] u [x, +inf), x != 0
        return value_from_inf_to_max(1.0 / v.leftVal);
    return value_from_to(1.0 / v.rightVal, 1.0 / v.leftVal);
}


// Implementations of interface functions

value value_precision(double x, double p) {
    const value result = {
        .empty = false,
        .real = false,
        .leftVal = fmin(x - x*(p/100), x + x*(p/100)),
        .rightVal = fmax(x + x*(p/100), x - x*(p/100)),
    };
    return result;
}

value value_from_to(double x, double y) {
    const value result = {
        .empty = false,
        .real = false,
        .leftVal = x,
        .rightVal = y,
    };
    return result;
}

value value_exact(double x) {
    const value result = {
        .empty = false,
        .real = false,
        .leftVal = x,
        .rightVal = x,
    };
    return result;
}

bool in_value(value v, double x) {
    switch (range_type(v)) {
        case EMPTY:
            return false;
        case REALS:
            return true;
        case INF_R:
            return le(x, v.rightVal);
        case L_INF:
            return ge(x, v.leftVal);
        case INF_RL_INF:
            return le(x, v.rightVal) || ge(x, v.leftVal);
        default:
            return ge(x, v.leftVal) && le(x, v.rightVal);
    };
}

double min_value(value v) {
    const range_type_t v_type = range_type(v);
    if (v_type == REALS)
        return -INFINITY;
    if (v_type == EMPTY)
        return NAN;
    if (v_type == L_INF || v_type == LR)
        return v.leftVal;
    return -INFINITY; // (-inf, a] u [b, inf)
}

double max_value(value v) {
    const range_type_t v_type = range_type(v);
    if (v_type == EMPTY)
        return NAN;
    if (v_type == INF_R || v_type == LR)
        return v.rightVal;
    return +INFINITY; // (-inf, a] u [b, inf) }
}

double mid_value(value v) {
    switch (range_type(v)) {
        case EMPTY:
        case REALS:
        case INF_RL_INF:
            return NAN;
        case INF_R:
            return -INFINITY;
        case L_INF:
            return INFINITY;
        default: // LR
            return (v.leftVal + v.rightVal) / 2.0;
    }
}

value add(value a, value b) {
    const range_type_t a_type = range_type(a);
    const range_type_t b_type = range_type(b);
    if (a_type == EMPTY || b_type == EMPTY) {
        return EMPTY_RANGE;
    }
    if (a_type == REALS || b_type == REALS || a_type == INF_R || b_type == INF_R || a_type == L_INF || b_type == L_INF) {
        if (
          ( a_type == REALS       ||  b_type == REALS      ) ||
          ( a_type == INF_R       &&  b_type == L_INF      ) ||
          ( a_type == L_INF       &&  b_type == INF_R      ) ||
          (has_inf(a)          &&  b_type == INF_RL_INF ) ||
          (a_type == INF_RL_INF   &&  has_inf(b) )
          ) {
            return REALS_RANGE;
        }
        if (a_type != INF_R && b_type != INF_R) {
            return value_from_min_to_inf(b.leftVal + a.leftVal);
        }
        // else a_type != L_INF && b_type != L_INF:
        return value_from_inf_to_max(b.rightVal + a.rightVal);
    }
    if (
      ( a_type == INF_RL_INF &&  b_type == INF_RL_INF                                   ) ||
      ( a_type == INF_RL_INF &&  ge(b.rightVal - b.leftVal, a.leftVal - a.rightVal)) ||
      ( b_type == INF_RL_INF && ge(a.rightVal - a.leftVal, b.leftVal - b.rightVal))
    ) {
        return REALS_RANGE;
    }
    else {
        return value_from_to(a.leftVal + b.leftVal, a.rightVal + b.rightVal);
    }
}

value subtract(value a, value b) {
    return add(a, opposite(b));
}

value multiply_one_inf(value a, value b) { // one end of a is INF, b does not contain the neighborhood of 0
    const range_type_t a_type = range_type(a);
    const range_type_t b_type = range_type(b);
    if (b_type == INF_RL_INF) { // then we additionally know that a does not contain the neighborhood of 0
        if (a_type == INF_R) {
            return value_from_to(a.rightVal * b.leftVal, a.rightVal * b.rightVal);
        }
        if (a_type == L_INF) {
            return value_from_to(a.leftVal * b.leftVal, a.leftVal * b.rightVal);
        }
    }
    if (a_type == INF_R && ge(b.leftVal, 0)) { // value b >= 0
        return value_from_inf_to_max(fmax(a.rightVal * b.leftVal, a.rightVal * b.rightVal));
    }
    if (a_type == INF_R && le(b.rightVal, 0)) { // b <= 0
        return value_from_min_to_inf(fmin(a.rightVal * b.leftVal, a.rightVal * b.rightVal));
    }
    if (a_type == L_INF && ge(b.leftVal, 0)) { // b >= 0
        return value_from_min_to_inf(fmin(a.leftVal * b.leftVal, a.leftVal * b.rightVal));
    }
    else { // a_type == L_INF && b <= 0
        return value_from_inf_to_max(fmax(a.leftVal * b.leftVal, a.leftVal * b.rightVal));
    }
}

value multiply_one_inf_rl_inf(value a, value b) { // abnormal interval (a) times normal interval (b) not containing the neighborhood of 0
    if (ge(b.leftVal, 0)) { // value b greater than or equal to zero
        double temp_left = fmin(a.leftVal * b.leftVal, a.leftVal * b.rightVal);
        double temp_right = fmax(a.rightVal * b.leftVal, a.rightVal * b.rightVal);
        return value_from_to(temp_left, temp_right);
    }
    // value b less than zero
    double temp_left = fmin(a.rightVal * b.rightVal, a.rightVal * b.leftVal);
    double temp_right = fmax(a.leftVal * b.leftVal, a.leftVal * b.rightVal);
    return value_from_to(temp_left, temp_right);
}

value multiply(value a, value b) {
    const range_type_t a_type = range_type(a);
    const range_type_t b_type = range_type(b);
    if (a_type == EMPTY || b_type == EMPTY) {
        return EMPTY_RANGE;
    }
    if (is_value_zero(a) || is_value_zero(b)) {
        return value_exact(0);
    }
    if ((is_zero_neighborhood(a) && has_inf(b)) || (is_zero_neighborhood(b) && has_inf(a))) {
        return REALS_RANGE;
    }
    if (a_type == L_INF && b_type == L_INF) {
        return value_from_min_to_inf(a.leftVal * b.leftVal);
    }
    if (a_type == INF_R && b_type == INF_R) {
        return value_from_min_to_inf(a.rightVal * b.rightVal);
    }
    if (a_type == INF_R && b_type == L_INF) {
        return value_from_inf_to_max(a.rightVal * b.leftVal);
    }
    if (a_type == L_INF && b_type == INF_R) {
        return value_from_inf_to_max(a.leftVal * b.rightVal);
    }
    if (a_type == L_INF || a_type == INF_R) {
        return multiply_one_inf(a, b);
    }
    if (b_type == L_INF || b_type == INF_R) {
        return multiply_one_inf(b, a);
    }
    /* at this point we know that if both intervals are in the form
     * (-inf, x] u [y, +inf), then x<=0 and y>=0 and x!=y
     * if they were not, REALS_RANGE would have been returned earlier */
    if (a_type == INF_RL_INF && b_type == INF_RL_INF) {
        const value temp = {
                .empty = false,
                .real = false,
                .leftVal = fmin(a.leftVal * b.leftVal, a.rightVal * b.rightVal),
                .rightVal = fmax(a.rightVal * b.leftVal, a.leftVal * b.rightVal)
        };
        if (le(temp.leftVal, temp.rightVal)) {
            return REALS_RANGE;
        }
        return temp;
    }
    if(a_type == INF_RL_INF)
        return multiply_one_inf_rl_inf(a, b);
    if(b_type == INF_RL_INF)
        return multiply_one_inf_rl_inf(b, a);
    // default case - [x, y]
    double possible_ends[4];
    possible_ends[0] = a.leftVal * b.leftVal;
    possible_ends[1] = a.leftVal * b.rightVal;
    possible_ends[2] = a.rightVal * b.leftVal;
    possible_ends[3] = a.rightVal * b.rightVal;
    return value_from_to(min_tab(possible_ends, 4), max_tab(possible_ends, 4));
}

value divide(value a, value b) {
    const range_type_t a_type = range_type(a);
    const range_type_t b_type = range_type(b);
    if (a_type == EMPTY || b_type == EMPTY || is_value_zero(b)) {
        return EMPTY_RANGE;
    }
    if (a_type == REALS || b_type == REALS) {
        return REALS_RANGE;
    }

    return multiply(a, reciprocal(b));
}