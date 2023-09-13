/*
 * client.c
 *
 *  Created on: Aug 23, 2023
 *      Author: yohanes
 */

#include "client.h"

#define MAX_RX_PACKET_LEN 2
#define TX_CLIENT_LEN 5

UART_HandleTypeDef *client_huart;

uint8_t rxbuff[MAX_RX_PACKET_LEN + 3];
uint8_t txbuff[TX_CLIENT_LEN] = {'i', 't', 's', 'o', 'k'};

uint8_t data[5];

uint8_t client_status;
uint8_t transmit_flag;
uint8_t callback_flag;
uint8_t restart_flag;
uint32_t waktu1;

void client_init(UART_HandleTypeDef *huart){
	client_huart = huart;
	HAL_UART_Receive_DMA(client_huart, rxbuff, MAX_RX_PACKET_LEN + 3);
}

void client_receive_restart(void){
	HAL_UART_DMAStop(client_huart);
//	static uint32_t last_restart = 0;
//	if (HAL_GetTick() < last_restart + 100)return;
//	last_restart = HAL_GetTick();
//	memset(rxbuff, 0, sizeof(rxbuff));
	HAL_UART_Receive_DMA(client_huart, rxbuff, MAX_RX_PACKET_LEN + 3);
	restart_flag++;
}

void client_receive_callback(UART_HandleTypeDef *huart){
	if (huart->Instance != client_huart->Instance)return;
	if (rxbuff[0] == 'i' && rxbuff[1] == 't' && rxbuff[2] == 's'){
		client_status = 1;
		memcpy(data, rxbuff, 5);
	}
	else {
		client_status = 0;
		client_receive_restart();
	}
	callback_flag++;
}

void client_transmit_routine(void){
	static uint32_t waktu = 0;
	if(HAL_GetTick() < waktu + 10)return;
	waktu = HAL_GetTick();
	txbuff[4]++;
	HAL_UART_Transmit_DMA(client_huart, txbuff, TX_CLIENT_LEN);
	transmit_flag++;
}
