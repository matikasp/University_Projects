#include <stdio.h>
#include <math.h>

#include "ary.h"

int main() {
  struct value one = value_exact(1);
  struct value zero = value_exact(0);

  printf("1=%d\n", in_value(multiply(one, zero), 0));
  
  printf("1=%d\n", in_value(multiply(zero, value_from_to(1., 10.)), 0));
  
  struct value small = value_from_to(0., 1.);
  
  printf("1=%d\n", in_value(multiply(zero, small), 0));
  
  struct value large = divide(one, small);
  printf("inf=%lf\n", mid_value(large));
  
  printf("1=%d\n", in_value(multiply(zero, large), 0));
  
  return 0;
}
