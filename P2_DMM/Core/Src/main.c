/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include  "main.h"
#include "a7.h"
#include <math.h>
void SystemClock_Config(void);
// global variables
uint16_t samples[SAMPLE_SIZE];
uint32_t idx = 0;
uint8_t values_collected_flag = 0;

// display variables
static uint32_t freq = 0;
static uint16_t DC_voltage = 0;
static uint16_t AC_RMS_voltage = 0;
static uint16_t maximum = 0;
static uint16_t minimum = 0;

int main(void)
{
	 HAL_Init();
	 SystemClock_Config();
	 ADC_init();
	 TIM2_Init();
	 UART_init();
	 __enable_irq();
 	// clear screen and reset cursor
  	USART_ESC_Code("[2J");
  	USART_ESC_Code("[H");

  	// print title to screen
  	USART_ESC_Code("[1m"); // bold
  	USART_ESC_Code("[4m"); // underline
  	USART_ESC_Code("[3;20H"); // move cursor line 3 column 20
  	UART_print("Digital Multimeter");
  	USART_ESC_Code("[0m"); // clear attributes
  	USART_ESC_Code("[6;5H"); // move cursor line 8 column 5
  	USART_ESC_Code("7"); // save cursor position


  	// different FSM states
  	typedef enum {
		ST_DC_RMS,
  		ST_FREQ_MAX_MIN,
		ST_DISPLAY,
		ST_WAIT,
  	} state_type;

  	state_type state = ST_WAIT;


 while (1)
 {
	  switch (state) {
	  	  // calculate DC and AC RMS values
	  	  case ST_DC_RMS:
	  		  get_DC_and_RMS();
	  		  state = ST_FREQ_MAX_MIN;
	  		  break;
	  	 // calculate frequency, minimum and maximum
	  	  case ST_FREQ_MAX_MIN:
	  		  get_frequency_max_min();
	  		  state = ST_DISPLAY;
	  		  break;
	  	// display calculated data to terminal
	  	  case ST_DISPLAY:
	  		  maximum = ADC_calibration(maximum);
	  		  minimum = ADC_calibration(minimum);
	  		  DC_voltage = ADC_calibration(DC_voltage);
	  		  AC_RMS_voltage = ADC_calibration(AC_RMS_voltage);
	  		  reset_cursor();
	  		  print_max_min();
	  		  print_voltages_frequency();
	  		  print_bargraph();
	  		  state = ST_WAIT;
	  		  break;
	  	// wait until the next batch of data has been collected
	  	  case ST_WAIT:
	  		  while (!values_collected_flag);
	  		  values_collected_flag = 0;
	  		  state = ST_DC_RMS;
	  		  break;
	  	// default to waiting
	  	  default:
	  		  state = ST_WAIT;
	  		  break;
	  }
 }
}

//------------ HELPER FUNCTIONS ------------------

/*
 * @brief resets cursor position to start
 */
void reset_cursor(void){
	USART_ESC_Code("8"); // restore cursor position
	USART_ESC_Code("[0J"); // clear screen from the cursor down
}

/*
 * @brief prints maximum and minimum voltages
 */
void print_max_min() {
	UART_print("Maximum voltage: ");
	ADC_print(maximum);
	UART_print(" | Minimum voltage: ");
	ADC_print(minimum);
	USART_ESC_Code("[7;5H"); // move cursor line 7 column 5
}

/*
 * @brief prints DC and AC voltages
 */
void print_voltages_frequency() {
	UART_print("DC voltage: ");
	ADC_print(DC_voltage);
	UART_print(" | AC RMS voltage: ");
	ADC_print(AC_RMS_voltage);
	UART_print(" | Frequency: ");
	print_freq();
}

/*
 * @brief prints DC and AC bar graph using the same axis
 */
void print_bargraph(void) {
	USART_ESC_Code ("[11;17H");
	USART_ESC_Code("[4m"); // underline
	UART_print("Voltage Bar Graphs");
  	USART_ESC_Code("[0m"); // clear attributes
	USART_ESC_Code("[13;5H"); // move cursor line 13 column 5
	UART_print("AC RMS  ");
	print_hashtags(AC_RMS_voltage);
	USART_ESC_Code("[14;5H"); // move cursor line 14 column 5
	UART_print("DC      ");
	print_hashtags(DC_voltage);
	USART_ESC_Code("[15;12H"); // move cursor line 15 column 12
	UART_print("|----|----|----|----|----|----|----|");
	USART_ESC_Code("[16;12H"); // move cursor line 16 column 12
	UART_print("0   0.5  1.0  1.5  2.0  2.5  3.0  3.5");
	USART_ESC_Code("[18;25H"); // move cursor line 18 column 25
	UART_print("Voltage(V)");
}

/*
 * @brief calculates number of hashtags printed with associated voltage value
 */
void print_hashtags(uint16_t num){
	char hashtag_buf[MAX_HASHTAGS];
	uint8_t idx = 0;

	// since num in in the thousands, divided by ten then round to nearest ones place
	num /= 10;
	uint8_t remainder = num%10;
	if (remainder >= 5) num += (10-remainder);
	else num -= remainder;
	num /= 10;

	// add num amount of hashtags to buffer for printing
	for (idx = 0; idx < num; idx++) {
		hashtag_buf[idx] = '#';
	}

	hashtag_buf[idx] = '\0';
	UART_print(hashtag_buf);
}

/*
 * @brief calculates the frequency, maximum ,and minimum voltages
 */
void get_frequency_max_min(){
	uint32_t avg_point = 0;
	maximum = 0;
	minimum = INT_MAX;
	// check how many times wave is at DC value to find frequency
	for (uint16_t i = 1; i < SAMPLE_SIZE; i++) {
		if ((samples[i - 1] > DC_voltage) && (samples[i] <= DC_voltage)){
			avg_point++;
		}
		// see if value being read is greater or smaller than the max and min
		if (samples[i] > maximum) maximum = samples[i];
		if (samples[i] < minimum) minimum = samples[i];

	}
	// divide by 3 since sampling for 3 periods
	freq = avg_point/PERIODS_SAMPLED;
}

/*
 * @brief calculates the DC and RMS values.
 * DC calculated by adding all values then dividing by sample size, since the DC
 * can be found by taking the average value of the waveform (if %50).
 * RMS found by squaring each value and adding them all up, then dividing by the
 * sample size, and then finally taking the square root of that value.
 */
void get_DC_and_RMS() {
	uint32_t mean = 0;
	uint64_t mean_squared = 0;
	for (uint16_t i = 0; i < SAMPLE_SIZE; i++) {
		mean += samples[i];
		mean_squared += (samples[i] * samples[i]);
	}
	DC_voltage = mean/SAMPLE_SIZE;
	AC_RMS_voltage = sqrt(mean_squared/SAMPLE_SIZE);
}

/*
 * @brief prints the waveform frequency.
 * NOTE: will print random frequency if measuring only DC voltage.
 */
void print_freq(void) {
	char freq_buf[7];
	uint8_t idx = 0;
	// ensures frequency is within range
	if (freq >= 1 && freq <= 1000) {
		int temp = freq;
		// Extract each digit
		while (temp > 0) {
			freq_buf[idx++] = '0' + (temp % 10);
			temp /= 10;
		}
		// Reverse the string
		for (int j = 0; j < idx / 2; j++) {
			char tempChar = freq_buf[j];
			freq_buf[j] = freq_buf[idx - j - 1];
			freq_buf[idx - j - 1] = tempChar;
		}
		// adds "Hz" to the end of the string
		freq_buf[idx++] = 'H';
		freq_buf[idx++] = 'z';
		freq_buf[idx] = '\0';
	}
	    // Error message for out of range
	    else {
	    	USART_ESC_Code("[8;5H");
	        UART_print("Frequency out of range, please input a waveform from 1 to 1000Hz");
	        return;
	    }
	UART_print(freq_buf);
	return;
}
 //--------------------------------------------------------


/**
  * @brief System Clock Configuration
  * @retval None
  * 80Mhz Clock
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
