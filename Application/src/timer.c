#include "main.h"
#include "tim.h"
#include "timer.h"

void timer_init() {
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim5);
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if(htim == &htim5) runtime_isr();
  if(htim == &htim2) stepper_isr();
}
