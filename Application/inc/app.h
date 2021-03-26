#ifndef APP_H
#define APP_H

#include <stdint.h>

#define RUNTIME_ISR_MICROSECONDS 100000
#define STEPPER_MICROSECONDS     1000000

extern volatile uint32_t time_now_millis;
extern volatile float time_now_float;

void loop() __attribute__ ((weak)); // weak declaration to be overriden
void setup() __attribute__ ((weak)); // weak declaration to be overriden
void reset_the_chip_do_not_do_this_unless_you_really_mean_it(void);
void update_time_now(uint32_t time_millis);

#endif // APP_H