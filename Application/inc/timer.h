#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void timer_init();

void runtime_isr() __attribute__ ((weak));
void stepper_isr() __attribute__ ((weak));

#endif // TIMER_H
