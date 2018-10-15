#include <stdint.h>
#include "math.h"
#include "hw.h"
#include "util.h"
 int divide(uint32_t dividend, uint32_t divisor)
{
  if (divisor == 0){
    log_str("tried dividing by zero");
    log_cr();
    PANIC();
  }
  uint32_t quotient = 0;
  while (dividend >= divisor){
    dividend -= divisor;
    quotient++;
  }
  return quotient;
}
 int modulo(uint32_t dividend, uint32_t divisor)
{
  int modulo = dividend - (divide(dividend, divisor) * dividend);
  return modulo;
}
