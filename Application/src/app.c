#include "main.h"
#include "usbd_cdc_if.h"

// Application specific includes
#include "timer.h"
#include "a4988.h"
#include "usb_serial.h"

volatile uint32_t time_now_millis;
volatile float time_now_float;

void setup() {
    timer_init();
    a4988_init();
    usb_serial_init();
    printf("setup completed!\n\r");
}


void loop() {
    // The loop function is mainly for handling stuff that doesn't have to 
    // happen at specific intervals.
    usb_serial_handle_tx_buffer_streaming();  
}


void runtime_isr() {
        string = "tock!\n\r";
        tick = true;
    printf("time is now %i seconds!\n\r", time_now_millis/1000);
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