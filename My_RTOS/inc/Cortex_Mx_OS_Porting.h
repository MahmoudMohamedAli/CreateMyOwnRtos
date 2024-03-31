/*
 * Cortex_Mx_OS_Porting.h
 *
 *  Created on: Mar 16, 2024
 *      Author: Mahmoud Elkot
 */

#ifndef INC_CORTEX_MX_OS_PORTING_H_
#define INC_CORTEX_MX_OS_PORTING_H_

#include "ARMCM3.h"


extern int _estack;   // Dependent on the MCU
extern int _eheap;

#define MainStackSize 3072  // 3K

#define Trigger_OS_PendSV() (SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk)

#define OS_SET_PSP(address) 		__asm volatile("mov r0,%0 \n\t msr psp,r0" : : "r"(address))
#define OS_GET_PSP(address) 		__asm volatile("mrs r0,psp \n\t mov %0,r0" : "=r"(address))

#define OS_SWITCH_SP_to_PSP()		__asm volatile("mrs r0, CONTROL \n\t orr r0,r0,#0x02 \n\t msr CONTROL, r0")
#define OS_SWITCH_SP_to_MSP()		__asm volatile("mrs r0, CONTROL \n\t mov r1,#0x05 \n\t and r0,r0,r1 \n\t msr CONTROL, r0")

// Set bit0 --> unprevilige
#define OS_SWITCH_TO_UNPRIVILIGE	__asm volatile("mrs r3,CONTROL \n\t orr r3,r3,#0x01 \n\t msr CONTROL,r3")
// Clear bit0 --> privilige
#define OS_SWITCH_TO_PRIVILIGE		__asm volatile("mrs r3,CONTROL \n\t lsr r3,r3,#0x1 \n\t lsl r3,r3,#0x1 \n\t msr CONTROL,r3")

void HW_Init(void);
void Start_Ticker(void);

#endif /* INC_CORTEX_MX_OS_PORTING_H_ */
