/*
 * lora.h
 *
 *  Created on: Aug 24, 2023
 *      Author: yohanes
 */

#ifndef INC_LORA_H_
#define INC_LORA_H_

#include "main.h"
#include "usart.h"

#define M0_Port 	GPIOA
#define M0_Pin 		GPIO_PIN_8
#define M1_Port		GPIOB
#define M1_Pin		GPIO_PIN_15
#define AUX_Port	GPIOB
#define AUX_Pin		GPIO_PIN_14

#define LORA_PIN_RECOVER 50

#define PERMANENT 0xC0
#define TEMPORARY 0xC2

typedef enum{
	LORA_NORMAL_MODE = 0,
	LORA_WAKEUP_MODE,
	LORA_POWERSAVING_MODE,
	LORA_PROGRAM_MODE
}LORA_Mode;

typedef struct{
	uint8_t addh;
	uint8_t addl;
	uint8_t speed;
	uint8_t channel;
	uint8_t option;
}lora_t;

typedef struct{
	float latitude;
	char NS;
	float longitude;
	char EW;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t valid;
	float altitude;
	char unit;
	uint8_t numOfSatelite;
}gps_t;

void lora_init(UART_HandleTypeDef *huart);
void lora_set_mode(LORA_Mode mode);
void lora_read_model_data();
void lora_receive_callback(UART_HandleTypeDef *huart);
void lora_set_ADDH(uint8_t val);
void lora_set_ADDL(uint8_t val);
void lora_set_speed(uint8_t val);
void lora_set_channel(uint8_t val);
void lora_set_option(uint8_t val);
void lora_save_param(uint8_t val);
void lora_serial_transmit(uint8_t *data, uint8_t len);
void lora_clear_buffer();
void lora_complete_task(unsigned long timeout);
void lora_receive_routine();
void lora_wireless_transmit_routine();
void lora_decode_gps(void);
#endif /* INC_LORA_H_ */
