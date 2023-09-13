/*
 * client.h
 *
 *  Created on: Aug 23, 2023
 *      Author: yohanes
 */

#ifndef INC_CLIENT_H_
#define INC_CLIENT_H_

#include "main.h"
#include "usart.h"

void client_init(UART_HandleTypeDef *huart);
void client_receive_restart(void);
void client_receive_callback(UART_HandleTypeDef *huart);
void client_transmit_routine(void);

#endif /* INC_CLIENT_H_ */
