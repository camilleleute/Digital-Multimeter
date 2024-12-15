/*
* a7.c
*
*  Created on: Nov 11, 2024
*      Author: camil
*/
#include "a7.h"
//------------  UART STUFF----------

//initializing USART2
void UART_init(void) {
	// enable GPIOA and USART clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	// AF7 configuration for PA2 and PA3
	GPIOA->AFR[0] &= ~((0xF << GPIO_AFRL_AFSEL2_Pos)|(0xF << GPIO_AFRL_AFSEL3_Pos)); // clear
	GPIOA->AFR[0] |= ((0x7 << GPIO_AFRL_AFSEL2_Pos)|(0x7 << GPIO_AFRL_AFSEL3_Pos)); // set
	//turn PA2 and PA3 on
	GPIOA->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
	GPIOA->MODER |= (2 << GPIO_MODER_MODE2_Pos) | (2 << GPIO_MODER_MODE3_Pos); // Set to AF mode (0b10)
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3);
	GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEED2 | GPIO_OSPEEDR_OSPEED3);
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3);
	// define word length, 8 bits
	USART2->CR1 &= ~(USART_CR1_M0 | USART_CR1_M1);
	// set baud rate
	USART2->BRR = BRR_num;
	// 1 stop bit
	USART2->CR2 &= ~(USART_CR2_STOP_1 | USART_CR2_STOP_0);
	// enable transmit and receive
	USART2->CR1 |= (USART_CR1_RE | USART_CR1_TE);
	// enable RX interrupts
	USART2->CR1 |= (USART_CR1_RXNEIE);
	//enable USART and interrupt
	USART2->CR1 |= USART_CR1_UE;
	NVIC->ISER[1] |= (1 << (USART2_IRQn & 0x1F));
}
/*
* @brief Prints custom message to terminal
*/
void UART_print(char *data) {
	// wait for TXE then write character and advance till null
   while (*data) {
       while (!(USART2->ISR & USART_ISR_TXE));
       USART2->TDR = *data++;
   }
}
/*
* @brief Auto prints ESC along with custom code
*/
void USART_ESC_Code(char* data) {
  while (!(USART2->ISR & USART_ISR_TXE));
  USART2->TDR = '\x1B';  // esc char
  while (*data) {
         while (!(USART2->ISR & USART_ISR_TXE));
         USART2->TDR = *data++;  // Send character and get the next one too
     }
}
//------------ ADC STUFF -----------
/*
* @brief Initializes ADC
*/
void ADC_init(void) {
	// Configure ADC clock
	  RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;
	  // run on system clock
	  ADC123_COMMON->CCR = (1<< ADC_CCR_CKMODE_Pos);
	  // Power up ADC and voltage regulator
	  ADC1->CR &= ~(ADC_CR_DEEPPWD);
	  ADC1->CR |= ADC_CR_ADVREGEN;
	  // wait 20 us
	  for (uint32_t i = 0; i<50000; i++);
	  // configure channel 5 for single ended mode
	  // Using channel 5 (PA0)
	  ADC1->DIFSEL &= ~ADC_DIFSEL_DIFSEL_5;
	  // Calibrate ADC
	  // ensure ADC disabled and single ended calibration
	  ADC1->CR &= ~(ADC_CR_ADEN | ADC_CR_ADCALDIF);
	  // start calibration
	  ADC1->CR |= ADC_CR_ADCAL;
	  // wait for calibration to finish
	  while (ADC1->CR & ADC_CR_ADCAL);
	  // enable ADC
	  // clear ADC Ready with a 1
	  ADC1->ISR |= (ADC_ISR_ADRDY);
	  ADC1->CR |= ADC_CR_ADEN;
	  while (!(ADC1->ISR & ADC_ISR_ADRDY));
	  // Configure sequence
	  // single channel 5, conversion size of 1
	  ADC1->SQR1 = (5 << ADC_SQR1_SQ1_Pos);
	  // Set ADC sample time for channel 5 to 2.5 ADC clock cycles
	  ADC1->SMPR1 &= ~ADC_SMPR1_SMP5;
	  ADC1->SMPR1 |= (7 << ADC_SMPR1_SMP5_Pos);
	  // configure 12 bit resolution, right align, single conversion mode
	  ADC1->CFGR = 0;
	  // Configure interrupts
	  ADC1->IER |= ADC_IER_EOCIE; //enable EOC interrupts
	  NVIC->ISER[0] |= (1 << (ADC1_2_IRQn & 0x1F));
	  // Configure GPIO for PA0
	  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; // enable pin clock
	  GPIOA->MODER &= ~(GPIO_MODER_MODE0);
	  GPIOA->MODER |= (GPIO_MODER_MODE0);
	  GPIOA->ASCR |= GPIO_ASCR_ASC0;
	  // start a conversion
	  ADC1->CR |= ADC_CR_ADSTART;
}


uint16_t ADC_calibration(uint16_t num) {
	// calibrate ADC
	num = (num * CALIBRATION_VALUE) + CALIBRATION_OFFSET;
	// round to 10s place
	num = ((num + 5) / 10) * 10;
	if (num >2670) num -= 20;
	return num;
}


void ADC_print(uint16_t num) {
	num /= 10;
	char voltage[8];
	uint8_t idx = 0;
	// get ones place
	int8_t ones = num / 100;
	// get tenth place
	int8_t tenths = (num % 100) / 10;
	// get hundredth place
	int8_t hundredth = num % 10;

	// put it all together
	voltage[idx++] = '0' + ones;
	voltage[idx++] = '.';
	voltage[idx++] = '0' + tenths;
	voltage[idx++] = '0' + hundredth;
	voltage[idx++] = 'V';
	voltage[idx++] = '\0';
	// print that
	UART_print(voltage);
}

/*
* @brief gets result from ADC
*/
void ADC1_2_IRQHandler(void) {
	if (ADC1->ISR & ADC_ISR_EOC){
		// Pull value from ADC and store it
		samples[idx++] = ADC1->DR;
		// reset when full
		if (idx == SAMPLE_SIZE) {
			idx = 0;
			values_collected_flag = 1;
		}
		ADC1->ISR &= ~ADC_ISR_EOC;
	}
}

//------------------------------ TIM2 Stuff ----------------------------------
/*
* @brief Initializes TIM2
*/
void TIM2_Init(void){
	// enable TIM2 clock
	RCC->APB1ENR1 |= (RCC_APB1ENR1_TIM2EN);
	// set TIM2 as an up counter
	TIM2->CR1 &= ~TIM_CR1_DIR;
	// Pre-scaler value to make each count equal to 1 us
	TIM2->PSC = 80 - 1;
   	// sample at 2kHz
   	TIM2->ARR = FIVEHUNDREDUS;
	// enable update event interrupt in TIM2 and CCR1 interrupts
	TIM2->DIER |=(TIM_DIER_UIE);
	// clear interrupt status register for update event and CCR1
	TIM2->SR &= ~(TIM_SR_UIF);
	// start timer
	TIM2->CR1 |= TIM_CR1_CEN;
	// enable TIM2 interrupts
	NVIC->ISER[0] = (1 <<(TIM2_IRQn & NVIC_REG_MASK));
	// Enable timer
	TIM2->CR1 |= TIM_CR1_CEN;
}

/*
* @brief resets adstart to fetch data
*/
void TIM2_IRQHandler(void){
	// ARR max reached
	if (TIM2->SR & TIM_SR_UIF) {
		// start ADC conversion
		ADC1->CR |= (ADC_CR_ADSTART);
		// clear flag
		TIM2->SR &= ~TIM_SR_UIF;
	}
}


