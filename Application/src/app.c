#include "main.h"
#include "usbd_cdc_if.h"

// Application specific includes
#include "timer.h"
#include "a4988.h"

volatile uint32_t time_now_millis;
volatile float time_now_float;

void setup() {
    HAL_Delay(5000);
    char *string = "Initialising the system!\n\r";
    CDC_Transmit_FS((uint8_t*)string, 27);    
    timer_init();
    a4988_init();
}


void loop() {
    // The loop function is mainly for handling stuff that doesn't have to 
    // happen at specific intervals.
    // HAL_Delay(1000);
    // char *string = "Hello World!\n\r";
    // CDC_Transmit_FS((uint8_t*)string, 15);    
}


void runtime_isr() {
    static bool tick = false;
    char *string;
    if(tick) {
        string = "tick!\n\r";
        tick = false;
    } else {
        string = "tock!\n\r";
        tick = true;
    }
    CDC_Transmit_FS((uint8_t*)string, 8);    
}


void stepper_isr() {
    
}


void update_time_now(uint32_t time_millis) {
  uwTick = time_millis;
  time_now_millis = uwTick;
  time_now_float = (float)uwTick * 0.001f;
}


void HAL_IncTick(void)
{
  uwTick += uwTickFreq;
  __DMB();
  time_now_millis = uwTick;
  time_now_float = (float)uwTick * 0.001f;
}


void reset_the_chip_do_not_do_this_unless_you_really_mean_it(void) {
  NVIC_SystemReset();
}