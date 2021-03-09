#ifndef A4988_H
#define A4988_H

#include <stdbool.h>

#include "main.h"

typedef struct {
    uint16_t pin;
    GPIO_TypeDef* port;
} pin_t;

typedef struct {
    pin_t enable;
    pin_t reset;
    pin_t sleep;
    pin_t ms1;
    pin_t ms2;
    pin_t ms3;
    pin_t dir;
    pin_t step;
} a4988_pins_t;

void a4988_init(void);
void a4988_dir(bool forward);
void a4988_step(void);

#endif // A4988_H
