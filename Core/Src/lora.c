/*
 * lora.c
 *
 *  Created on: Aug 24, 2023
 *      Author: yohanes
 */

#include "lora.h"

#define LORA_SERIAL_LEN 6

UART_HandleTypeDef *lora_huart;

lora_t lora;

uint8_t txdummy[5] = {'i','t','s','o','k'};
uint8_t rxdummy[5];

uint8_t lora_txbuff[LORA_SERIAL_LEN];
uint8_t lora_rxbuff[LORA_SERIAL_LEN];
uint8_t lora_param[LORA_SERIAL_LEN];
uint8_t lora_dummy_rxbuff;
uint8_t aux;
uint8_t lora_rxbuf[23];

uint8_t rxloraflag;
uint8_t lora_rxstat;

uint8_t lora_callback;

gps_t gpsData;

void lora_init(UART_HandleTypeDef *huart){
	aux = HAL_GPIO_ReadPin(AUX_Port, AUX_Pin);
	lora_huart = huart;
	lora_read_model_data();
	lora_set_ADDH(0);
	lora_set_ADDL(0);
	lora_set_speed(0b00011000);
	lora_set_channel(0b00000110);
	lora_set_option(0b01000100);
	lora_save_param(PERMANENT);
	HAL_Delay(500);
}

void lora_set_mode(LORA_Mode mode){
	HAL_Delay(LORA_PIN_RECOVER);
	switch(mode){
	case LORA_NORMAL_MODE:
		HAL_GPIO_WritePin(M0_Port, M0_Pin, 0);
		HAL_GPIO_WritePin(M1_Port, M1_Pin, 0);
		break;
	case LORA_WAKEUP_MODE:
		HAL_GPIO_WritePin(M0_Port, M0_Pin, 1);
		HAL_GPIO_WritePin(M1_Port, M1_Pin, 0);
		break;
	case LORA_POWERSAVING_MODE:
		HAL_GPIO_WritePin(M0_Port, M0_Pin, 0);
		HAL_GPIO_WritePin(M1_Port, M1_Pin, 1);
		break;
	case LORA_PROGRAM_MODE:
		HAL_GPIO_WritePin(M0_Port, M0_Pin, 1);
		HAL_GPIO_WritePin(M1_Port, M1_Pin, 1);
		break;
	}
	HAL_Delay(LORA_PIN_RECOVER);
	lora_clear_buffer();
	lora_complete_task(200);
}

void lora_read_model_data(){
	lora_txbuff[0] = 0xc1;
	lora_txbuff[1] = 0xc1;
	lora_txbuff[2] = 0xc1;

	memset(lora_param, 0, 6);

	lora_set_mode(LORA_PROGRAM_MODE);

	HAL_UART_Transmit_DMA(lora_huart, lora_txbuff, 3);
	HAL_UART_Receive_DMA(lora_huart, &lora_param, LORA_SERIAL_LEN);

	lora_set_mode(LORA_NORMAL_MODE);
}

void lora_receive_callback(UART_HandleTypeDef *huart){
	if (huart->Instance != lora_huart->Instance)return;
	lora_callback++;
	if (lora_rxbuf[0] == 'i'&& lora_rxbuf[1] == 't'&& lora_rxbuf[2] == 's'){
		lora_rxstat = 1;
		lora_decode_gps();
	}
	else{
		lora_rxstat = 0;
		HAL_UART_DMAStop(lora_huart);
		memset(lora_rxbuf,0,23);
		HAL_UART_Receive_DMA(lora_huart, lora_rxbuf, 23);
	}

}

void lora_set_ADDH(uint8_t val){
	lora.addh = val;
}

void lora_set_ADDL(uint8_t val){
	lora.addl = val;
}

void lora_set_speed(uint8_t val){
	lora.speed = val;
}

void lora_set_channel(uint8_t val){
	lora.channel = val;
}

void lora_set_option(uint8_t val){
	lora.option = val;
}

void lora_save_param(uint8_t val){
	lora_txbuff[0] = val;
	lora_txbuff[1] = lora.addh;
	lora_txbuff[2] = lora.addl;
	lora_txbuff[3] = lora.speed;
	lora_txbuff[4] = lora.channel;
	lora_txbuff[5] = lora.option;

	lora_serial_transmit(lora_txbuff, LORA_SERIAL_LEN);

	lora_read_model_data();
}

void lora_serial_transmit(uint8_t *data, uint8_t len){
	lora_set_mode(LORA_PROGRAM_MODE);
	HAL_UART_Transmit_DMA(lora_huart, data, len);
	lora_set_mode(LORA_NORMAL_MODE);
}

void lora_clear_buffer(){
	unsigned long time = HAL_GetTick();
	while(HAL_GetTick() < time + 500){
		HAL_UART_Receive_DMA(lora_huart, lora_dummy_rxbuff, 1);
	}
}

void lora_complete_task(unsigned long timeout){
	unsigned long time = HAL_GetTick();
	aux = HAL_GPIO_ReadPin(AUX_Port, AUX_Pin);
	if (((unsigned long) (time + timeout)) == 0){
		time = 0;
	}
	while (HAL_GPIO_ReadPin(AUX_Port, AUX_Pin) == 0){
		HAL_Delay(LORA_PIN_RECOVER);
		if (HAL_GetTick() > time + timeout)break;
	}
	aux = HAL_GPIO_ReadPin(AUX_Port, AUX_Pin);
}

void lora_receive_routine(){
	aux = HAL_GPIO_ReadPin(AUX_Port, AUX_Pin);
	static uint32_t rxdelay = 0;
	if(HAL_GetTick() < rxdelay + 1000)return;
	rxdelay = HAL_GetTick();
	HAL_UART_Receive_DMA(lora_huart, lora_rxbuf, 23);
	aux = HAL_GPIO_ReadPin(AUX_Port, AUX_Pin);
	rxloraflag++;
}

void lora_wireless_transmit_routine(){
//	memcpy(lora_data_tx.dummy, dummy, 5);
	aux = HAL_GPIO_ReadPin(AUX_Port, AUX_Pin);
	static uint32_t delay = 0;
	if(HAL_GetTick() < delay + 2000)return;
	delay = HAL_GetTick();
	HAL_UART_Transmit_DMA(lora_huart, txdummy, 5);
	txdummy[3]++;
	aux = HAL_GPIO_ReadPin(AUX_Port, AUX_Pin);
//	lora_complete_task(1000);
}
/*
memset(lora_tx_gps+3,0,16);
memcpy(lora_tx_gps+3, &coordinates[0],4); //argumen ke-2 memcpy hanya works untuk 8bit, kalau lebih harus pake tanda & (yang diambil alamat memorinya)
memcpy(lora_tx_gps+7, &gpsData.ggastruct.lcation.NS,1);
memcpy(lora_tx_gps+8, &coordinates[1],4);
memcpy(lora_tx_gps+12, &gpsData.ggastruct.lcation.EW,1);
//	memcpy(lora_tx_gps+13, (int8_t*)gpsData.ggastruct.tim.hour,1);
memcpy(lora_tx_gps+13, &h,1);
memcpy(lora_tx_gps+14, &m,1);
memcpy(lora_tx_gps+15, &s,1);
memcpy(lora_tx_gps+16, &v,1);
//	memcpy(lora_tx_gps+14, (unt8_t*)&gpsData.ggastruct.tim.min,1);
//	memcpy(lora_tx_gps+15, (uint8_t*)&gpsData.ggastruct.tim.sec,1);
//	memcpy(lora_tx_gps+16, (int8_t*)gpsData.ggastruct.isfixValid,1);
memcpy(lora_tx_gps+17, &gpsData.ggastruct.alt.altitude,4);
memcpy(lora_tx_gps+21, &gpsData.ggastruct.alt.unit,1);
//	memcpy(lora_tx_gps+22, (int8_t*)gpsData.ggastruct.numofsat,1);
memcpy(lora_tx_gps+22, &n,1);
*/
void lora_decode_gps(void){
	memcpy(&gpsData.latitude,lora_rxbuf+3, 4);
	memcpy(&gpsData.longitude,lora_rxbuf+8, 4);
//	gpsData.latitude = ((lora_rxbuf[3]<<24) + (lora_rxbuf[4]<<16) + (lora_rxbuf[5]<<8) + (lora_rxbuf[6]<<0));
	gpsData.NS = lora_rxbuf[7];
//	gpsData.longitude = (lora_rxbuf[8]<<24 | lora_rxbuf[9]<<16 | lora_rxbuf[10]<<8 | lora_rxbuf[11]<<0);
	gpsData.EW = lora_rxbuf[12];
	gpsData.hour = lora_rxbuf[13];
	gpsData.min = lora_rxbuf[14];
	gpsData.sec = lora_rxbuf[15];
	gpsData.valid = lora_rxbuf[16];
//	gpsData.altitude = (lora_rxbuf[17]<<24 | lora_rxbuf[18]<<16 | lora_rxbuf[19]<<8 | lora_rxbuf[20]<<0);
	memcpy(&gpsData.altitude,lora_rxbuf+17, 4);
	gpsData.unit = lora_rxbuf[21];
	gpsData.numOfSatelite = lora_rxbuf[22];

}
