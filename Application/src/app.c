#include "main.h"
#include "usbd_cdc_if.h"

volatile uint32_t time_now_millis;
volatile float time_now_float;

void setup() {
    HAL_Delay(5000);
    char *string = "Initialising the system!\n\r";
    CDC_Transmit_FS((uint8_t*)string, 27);    
}


void loop() {
    HAL_Delay(1000);
    char *string = "Hello World!\n\r";
    CDC_Transmit_FS((uint8_t*)string, 15);    
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