/**
 * \file
 *         Implementation of architecture-specific clock functionality
 */

#include "clock-arch.h"

volatile unsigned int *reg_32ksyncnt_cr = (volatile unsigned int *) TIMER_ADDR;

/*---------------------------------------------------------------------------*/
clock_time_t clock_time(void)
{
  // Assume clock memory mapping has been properly initialized

  return *reg_32ksyncnt_cr;
}
/*---------------------------------------------------------------------------*/
