#include <math.h>
#include "main.h"
#include "usbd_cdc_if.h"

// Application specific includes
#include "timer.h"
#include "a4988.h"
#include "usb_serial.h"

volatile uint32_t time_now_millis;
volatile float time_now_float;

void setup() {
    usb_serial_init();
    // HAL_Delay(1000);
    a4988_init();
    printf("setup completed!\n\r");
    a4988_set_step_divisions(16);
    a4988_velocity_mode(400.0f);
    timer_init();
}


void loop() {
    // The loop function is mainly for handling stuff that doesn't have to 
    // happen at specific intervals.
    usb_serial_handle_tx_buffer_streaming();  
}


void runtime_isr() {
    // // printf("runtime_isr\r\n");
    // static float rpm = 0.0f;
    // // const float dt = 1.0f;
    // const float rpm_limit = 300.0f;
    // if (rpm < rpm_limit) {
    //     // float new_rpm = rpm + rpm_limit / dt * (float)(1000000 / RUNTIME_ISR_MICROSECONDS);
    //     printf("%.3f\n\r", (double)rpm);
    //     rpm += 1.0f;
    // } else {
    //     // printf("%.3f\n\r", (double)rpm);
    // }
    // a4988_velocity_mode(rpm);    
    // // static bool run_once = false;
    // // static float radians = M_PI / 2.0;
    // // if(time_now_millis/1000 > 2 && !run_once) {
    // //     a4988_step_rpm(a4988_get_steps_from_radians(radians), 60.0f);
    // //     // run_once = true;
    // // }
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