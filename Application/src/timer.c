#include "main.h"
#include "tim.h"
#include "timer.h"

void timer_init() {
  HAL_TIM_Base_Start_IT(&htim2);
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if(htim == &htim2) runtime_isr();
}
