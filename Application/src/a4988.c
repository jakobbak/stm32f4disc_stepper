#include <math.h>
#include "app.h"
#include "tim.h"
#include "timer.h"
#include "a4988.h"
#include "usbd_cdc_if.h"


a4988_t a4988;

// #define A4988_DEBUG

void a4988_init_pins(void) {
    a4988.pins.enable.pin   = A4988_ENABLE_Pin;
    a4988.pins.enable.port  = A4988_ENABLE_GPIO_Port;
    a4988.pins.reset.pin    = A4988_RESET_Pin;
    a4988.pins.reset.port   = A4988_RESET_GPIO_Port;
    a4988.pins.sleep.pin    = A4988_SLEEP_Pin;
    a4988.pins.sleep.port   = A4988_SLEEP_GPIO_Port;
    a4988.pins.ms1.pin      = A4988_MS1_Pin;
    a4988.pins.ms1.port     = A4988_MS1_GPIO_Port;
    a4988.pins.ms2.pin      = A4988_MS2_Pin;
    a4988.pins.ms2.port     = A4988_MS2_GPIO_Port;
    a4988.pins.ms3.pin      = A4988_MS3_Pin;
    a4988.pins.ms3.port     = A4988_MS3_GPIO_Port;
    a4988.pins.dir.pin      = A4988_DIR_Pin;
    a4988.pins.dir.port     = A4988_DIR_GPIO_Port;
    a4988.pins.step.pin     = A4988_STEP_Pin;
    a4988.pins.step.port    = A4988_STEP_GPIO_Port;
}

static void high(pin_t pin) {
    HAL_GPIO_WritePin(pin.port, pin.pin, GPIO_PIN_SET);
}

static void low(pin_t pin) {
    HAL_GPIO_WritePin(pin.port, pin.pin, GPIO_PIN_RESET);
}

static inline void start_pwm_output(void) {
    low(a4988.pins.enable);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
#ifdef A4988_DEBUG
    printf("HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1)\n\r");
#endif
}

static inline void stop_pwm_output(void) {
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
    high(a4988.pins.enable);
#ifdef A4988_DEBUG
    printf("HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1)\n\r");
#endif
}

static inline void set_pwm_reg(float rpm) {
    float steps_per_second = rpm / 60.0f * (float)a4988.num_microsteps;
    a4988.microseconds_per_step = (uint32_t)(1000000.0f / steps_per_second);// * 84.0f);
    __HAL_TIM_SET_AUTORELOAD(&htim2, a4988.microseconds_per_step - 1);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, a4988.microseconds_per_step / 2 - 1);
#ifdef A4988_DEBUG
    printf("rpm = %.3f, CCR1 + 1 = %u, ARR + 1 = %u\n\r",
            (double)a4988.rpm,
            (unsigned int)htim2.Instance->CCR1 + 1,
            (unsigned int)htim2.Instance->ARR + 1
          );
#endif
}


static void set_forward(bool forward) {
    a4988.forward = forward;
    if(a4988.forward) high(a4988.pins.dir);
    else low(a4988.pins.dir);
}


static void set_rpm(float rpm) {
    if(rpm < 0.0f) {
        set_forward(false);
        rpm = -rpm;
    } else {
        set_forward(true);
    }
    a4988.rpm = rpm;
    if(rpm == 0.0f) {
        stop_pwm_output();
#ifdef A4988_DEBUG
        printf("rpm = %.3f, CCR1 + 1 = %u, ARR + 1 = %u\n\r",
                (double)a4988.rpm,
                (unsigned int)htim2.Instance->CCR1 + 1,
                (unsigned int)htim2.Instance->ARR + 1
              );
#endif
        return; // avoid divide by zero below
    }
    set_pwm_reg(rpm);
}

void a4988_init(void) {
    a4988_init_pins();
    low(a4988.pins.enable); // enabled at logic low
    high(a4988.pins.reset); // in reset state at logic low
    high(a4988.pins.sleep); // in sleep mode at logic low
    low(a4988.pins.ms1);
    low(a4988.pins.ms2);
    low(a4988.pins.ms3);
    a4988.velocity_mode = false;
    a4988.num_full_steps = 200;
    a4988_set_step_divisions(1);
    a4988_set_forward(true);
    a4988.step = 0;
    a4988.target = 0;
    a4988.rpm = 0;
    a4988.velocity_mode = false;
}


void a4988_set_forward(bool forward) {
    set_forward(forward);
}


void a4988_set_rpm(float rpm) {
    set_rpm(rpm);
}


bool a4988_step(uint32_t steps) {
    if(a4988.rpm == 0.0f) return false;
    a4988_step_rpm(steps, a4988.rpm);
    return true;
}


void a4988_step_rpm(uint32_t steps, float rpm) {
#ifdef A4988_DEBUG
    printf("a4988_step_rpm(%u, %.3f) called\n\r", (unsigned int)steps, (double)rpm);
#endif
    a4988.velocity_mode = false;
    a4988.target += steps;
    set_rpm(rpm);
    if (a4988.target - a4988.step < 0) {
        set_forward(false);
    } else {
        set_forward(true);
    }
    start_pwm_output();
}


void a4988_velocity_mode(float rpm) {
#ifdef A4988_DEBUG
    printf("velocity_mode(%.3f) called\n\r", (double)rpm);
#endif
    a4988.velocity_mode = true;
    set_rpm(rpm);
    if(rpm != 0.0f) start_pwm_output();
}


void stepper_isr() {
    if(a4988.rpm != 0.0f) {
        if(a4988.forward)   a4988.step++;
        else                a4988.step--;
        if(a4988.velocity_mode) {
#ifdef A4988_DEBUG
            printf("stepper_isr() called, velocity_mod, rpm = %.3f\n\r", (double)a4988.rpm);
#endif
            a4988.target = a4988.step;
        } else {
#ifdef A4988_DEBUG
            printf("stepper_isr() called, step = %i, target = %i\n\r", (unsigned int)a4988.step, (unsigned int)a4988.target);
#endif
            if(a4988.step == a4988.target) {
                stop_pwm_output();
            }
        }        
    } else {
        // TODO: CONSIDER APPROPRIATE IMPLEMENTATION
#ifdef A4988_DEBUG
        printf("stepper isr() called, rpm = 0.0f\n\r");
#endif
    }
}


bool a4988_set_step_divisions(uint32_t div) {
#ifdef A4988_DEBUG
    printf("a4988_set_step_divisions(%u) called\n\r", (unsigned int)div);
#endif
    if(div == 1 || div == 2 || div == 4 || div == 8 || div == 16) {
        switch (div) {  // can only be 1, 2, 4, 8 or 16
            case 1:
            low(a4988.pins.ms1);
            low(a4988.pins.ms2);
            low(a4988.pins.ms3);
            break;
            case 2:
            high(a4988.pins.ms1);
            low(a4988.pins.ms2);
            low(a4988.pins.ms3);
            break;
            case 4:
            low(a4988.pins.ms1);
            high(a4988.pins.ms2);
            low(a4988.pins.ms3);
            break;
            case 8:
            high(a4988.pins.ms1);
            high(a4988.pins.ms2);
            low(a4988.pins.ms3);
            break;
            case 16:
            high(a4988.pins.ms1);
            high(a4988.pins.ms2);
            high(a4988.pins.ms3);
            break;
            default: return false;
        }
        a4988.step_divisions = div;
        a4988.num_microsteps = a4988.step_divisions * a4988.num_full_steps;
        set_rpm(a4988.rpm);
        return true;
    } else {
        return false;
    }
}


uint32_t a4988_get_target(void) {
    return a4988.target;
}


uint32_t a4988_get_step(void) {
    return a4988.step;
}


float a4988_get_rpm(void) {
    return a4988.rpm;
}

uint32_t a4988_get_steps_from_radians(float rad) {
    return (float)(a4988.num_microsteps / M_PI / 2.0 * (double)rad);
}


