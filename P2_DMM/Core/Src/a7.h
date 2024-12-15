/*
 * a7.h
 *
 *  Created on: Nov 14, 2024
 *      Author: camil
 */

#ifndef SRC_A7_H_
#define SRC_A7_H_
#include "stm32l4xx_hal.h"
#define BRR_num 694
#define INT_MAX 65535
#define MAX_TIME 0xFFFFFFFF
#define NVIC_REG_MASK 0x1F
#define SAMPLE_SIZE 6000
#define FIVEHUNDREDUS 500
#define PERIODS_SAMPLED 3

#define MAX_HASHTAGS 35
#define CALIBRATION_VALUE 0.814
#define CALIBRATION_OFFSET 0.433
void print_bargraph(void);
void print_hashtags(uint16_t);
uint16_t ADC_calibration(uint16_t);
void reset_cursor(void);
void print_max_min(void);
void print_voltages_frequency(void);
void print_freq(void);
void TIM2_Init(void);
void ADC_init(void);
void ADC_print(uint16_t);
void get_frequency_max_min(void);
void get_DC_and_RMS(void);
void USART_ESC_Code(char*);
void UART_print(char*);
void UART_init(void);
#endif /* SRC_A7_H_ */
