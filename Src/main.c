/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */


#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "STM32F103x8.h"
#include "stm32f103x8_gpio_driver.h"
#include "lcd.h"
#include "keypad.h"
#include "stm32f103x8_EXTI_driver.h"
#include "stm32f103x8_USART_driver.h"
#include "stm32f103x8_SPI_driver.h"
#include "stm32f103x8_I2C_driver.h"
#include "I2C_Slave_EEPROM.h"
#include "core_cm3.h"
#include "Schedular.h"

Task_ref Task1, Task2, Task3, Task4;
uint8_t	 Task1LED, Task2LED, Task3LED, Task4LED;
Mutex_ref MUTEX1;

uint8_t array[3] = {1,2,3};

void Task_1(void);

void Task_2(void);

void Task_3(void);

void Task_4(void);

int main(void)
{
	MYRTOS_errorID retval = noError;
	/* HW_Init (Initialize Clock tree, Reset Controller) */
	HW_Init();
	retval |= MYRTOS_Init();

	Task1.Stack_Size = 1024;
	Task1.pf_TaskEntry = Task_1;
	Task1.Priority = 10;
	Task1.AutoStart = Autostart_Enabled;
	strcpy(Task1.TaskName,"Task_1");

	Task2.Stack_Size = 1024;
	Task2.pf_TaskEntry = Task_2;
	Task2.Priority = 8;
	Task2.AutoStart = Autostart_Disabled;
	strcpy(Task2.TaskName ,"Task_2");

	Task3.Stack_Size = 1024;
	Task3.pf_TaskEntry = Task_3;
	Task3.Priority = 1;
	Task3.AutoStart = Autostart_Disabled;
	strcpy(Task3.TaskName , "Task_3");

	Task4.Stack_Size = 1024;
	Task4.pf_TaskEntry = Task_4;
	Task4.Priority = 3;
	Task4.AutoStart = Autostart_Disabled;
	strcpy(Task4.TaskName ,"Task_4");

	retval |= MYRTOS_Create_Task(&Task1);
	retval |= MYRTOS_Create_Task(&Task2);
	retval |= MYRTOS_Create_Task(&Task3);
	retval |= MYRTOS_Create_Task(&Task4);

	MUTEX1.PayloadSize = 3;
	MUTEX1.pPayload = array;
	MUTEX1.PriorityCeiling.state = PriorityCeiling_disabled;
	MUTEX1.PriorityCeiling.Ceiling_Priority = 2;
	strcpy(MUTEX1.MutexName, "Mutex Shared T1_T4");

	MYRTOS_StartOS();

    while(1);
}

void Task_1(void){
	uint8_t count = 0;
	while(1){
		/* Taskl Code */
			Task1LED ^= 1;
			count++;
		if(count == 100){
			MYRTOS_AcquireMutex(&MUTEX1, &Task1);
			MYRTOS_ActivateTask(&Task2);
		}
		else if(count == 200){
			count = 0;
			MYRTOS_ReleaseMutex(&MUTEX1);
		}
	}
}

void Task_2(void){
	uint8_t count = 0;
	while(1){
		/* Task2 Code */
		Task2LED ^= 1;
		count++;
		if(count == 100){
			MYRTOS_ActivateTask(&Task3);
		}
		else if(count == 200){
			count = 0;
			MYRTOS_TerminateTask(&Task2);
		}
	}
}

void Task_3(void){
	uint8_t count = 0;
	while(1){
		/* Task3 Code */
		Task3LED ^= 1;
		count++;
		if(count == 100){
			MYRTOS_ReleaseMutex(&MUTEX1);
			MYRTOS_ActivateTask(&Task4);
		}
		else if(count == 200){
			count = 0;
			MYRTOS_TerminateTask(&Task3);
		}
	}
}

void Task_4(void){
	uint8_t count = 0;
	while(1){
		/* Task4 Code */
		Task4LED ^= 1;
		count++;
		if(count == 3){
			MYRTOS_AcquireMutex(&MUTEX1, &Task4);
		}
		else if(count == 200){
			count = 0;
			MYRTOS_ReleaseMutex(&MUTEX1);
			MYRTOS_TerminateTask(&Task4);
		}
	}
}
