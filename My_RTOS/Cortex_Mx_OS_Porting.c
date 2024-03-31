/*
 * Cortex_Mx_OS_Porting.c
 *
 *  Created on: Mar 16, 2024
 *      Author: Mahmoud ELkot
 */

// Enabling all faults for possible errors (Accessing memories etc.)
#include "core_cm3.h"

void HardFault_Handler()
{
	while(1);
}

void MemManage_Handler()
{
	while(1);
}

void BusFault_Handler()
{
	while(1);
}

void UsageFault_Handler()
{
	while(1);
}


// Consider it as C function, but don't push anything in stack, it is assembly, force the compiler to not add anything
__attribute ((naked))  void SVC_Handler()
{
	// Check EXC_Return to see if you were MSP or PSP
	__asm("tst lr,#4 \n\t"
			"ITE EQ \n\t"
			"mrseq r0,MSP \n\t"
			"mrsne r0,PSP \n\t"
			"B MYRTOS_SVC_services");
}



void HW_Init()
{
	// Initialize clock tree (RCC --> SysTick Timer & CPU) 8MHz
	// 8 MHz (By default)
	// 1 count = 0.125 us
	// X count -> 1ms
	// X = 8000 counts
	/* Decrease PendSV interrupt priority to be smaller or equal to systick timer */
	__NVIC_SetPriority(PendSV_IRQn, 15);
}

void Start_Ticker(void){
	SysTick_Config(8000);
}

