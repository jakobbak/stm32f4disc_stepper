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

typedef struct {
    a4988_pins_t pins;
    uint32_t num_full_steps;
    uint32_t step_divisions;
    uint32_t num_microsteps;    
    bool forward;
    int32_t step;
    int32_t target;
    float rpm;
    bool velocity_mode;
    uint32_t microseconds_per_step;
} a4988_t;

void a4988_init(void);
bool a4988_step(uint32_t steps);
void a4988_step_rpm(uint32_t steps, float rpm);
void a4988_velocity_mode(float rpm);
void a4988_set_forward(bool forward);
bool a4988_set_step_divisions(uint32_t div); // can only be 1, 2, 4, 8 or 16
void a4988_set_velocity(float vel);
uint32_t a4988_get_target(void);
uint32_t a4988_get_step(void);
void a4988_set_rpm(float rpm);
float a4988_get_rpm(void);
uint32_t a4988_get_steps_from_radians(float rad);

#endif // A4988_H
