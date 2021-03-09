// #include "app.h"
#include "tim.h"
#include "a4988.h"

a4988_pins_t a4988;

void a4988_init_pins(void) {
    a4988.enable.pin   = A4988_ENABLE_Pin;
    a4988.enable.port  = A4988_ENABLE_GPIO_Port;
    a4988.reset.pin    = A4988_RESET_Pin;
    a4988.reset.port   = A4988_RESET_GPIO_Port;
    a4988.sleep.pin    = A4988_SLEEP_Pin;
    a4988.sleep.port   = A4988_SLEEP_GPIO_Port;
    a4988.ms1.pin      = A4988_MS1_Pin;
    a4988.ms1.port     = A4988_MS1_GPIO_Port;
    a4988.ms2.pin      = A4988_MS2_Pin;
    a4988.ms2.port     = A4988_MS2_GPIO_Port;
    a4988.ms3.pin      = A4988_MS3_Pin;
    a4988.ms3.port     = A4988_MS3_GPIO_Port;
    a4988.dir.pin      = A4988_DIR_Pin;
    a4988.dir.port     = A4988_DIR_GPIO_Port;
    a4988.step.pin     = A4988_STEP_Pin;
    a4988.step.port    = A4988_STEP_GPIO_Port;    
}

static void set(pin_t pin) {
    HAL_GPIO_WritePin(pin.port, pin.pin, GPIO_PIN_SET);
}

static void reset(pin_t pin) {
    HAL_GPIO_WritePin(pin.port, pin.pin, GPIO_PIN_RESET);
}

void a4988_init(void) {
    a4988_init_pins();
    reset(a4988.enable); // enabled at logic low
    set(a4988.reset); // in reset state at logic low
    set(a4988.sleep); // in sleep mode at logic low
    reset(a4988.ms1);
    reset(a4988.ms2);
    reset(a4988.ms3);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

void a4988_set_dir(bool forward) {
    if(forward) set(a4988.dir);
    else reset(a4988.dir);
}

void a4988_step() {
    static bool low = false;
    if(low) {
        set(a4988.step);
        low = false;
    } else {
        reset(a4988.step);
        low = true;
    }
}

