/*
 * Schedular.c
 *
 *  Created on: Mar 16, 2024
 *      Author: Mahmoud Elkot
 */

#include "Schedular.h"
#include "My_RTOS_FIFO.h"

#include "string.h"


uint8_t IdleTaskLED, SysTickLED;

FIFO_Buffer_t ReadyQueue;
Task_ref *ReadyQueueFIFO[MAX_NO_TASKS];

Task_ref MYRTOS_IdleTask;


// OS states (in .c not .h to not being seen by the user)

struct{
	Task_ref* OS_Tasks[MAX_NO_TASKS];  // Tasks(schedular) table (Can hold up to 100 tasks)
	uint32_t _S_MSP;   // Pointed to the start of the SRAM (From startup)
	uint32_t _E_MSP;   // Then this will act as the size of the stack
	uint32_t PSP_Task_Locator; //pointer to stack for new task that will be created "Updated as new task Added."
	uint32_t ActiveTasksNum;   // the number of task which is active state.
	Task_ref* CurrentTaskExecuted;  //the current which is running
	Task_ref* NextTaskTobeExecuted; //the Next Task which should be run
	enum{
		OS_Suspend,  //operating system is suspended.
		OS_Running,  //operating system is running.
		OS_Error
	}OS_Modes_ID;
}OS_Control;

typedef enum{
	SVC_ActivateTask,
	SVC_TerminateTask,
	SVC_TaskWaitingTime
}SVC_ID;

static void MyRTOS_Idle_Task();
static void MYRTOS_UpdateSchedulerTable(void);
static void OS_SVC_Set(int SVC_ID);
static void MYRTOS_Create_MainStack();
static void MYRTOS_DecideNextTask(void);
static void MYRTOS_BubbleSort(void);
static void MYRTOS_Create_Stack(Task_ref* taskReference);
static void MYRTOS_Update_TasksWaitingTime(void);

static void OS_SVC_Set(int SVC_ID)
{
	switch(SVC_ID)
	{
	case SVC_ActivateTask:  // Activate task
		__asm("svc #0x01");
		break;

	case SVC_TerminateTask:  // Terminate task
		__asm("svc #0x02");
		break;

	case SVC_TaskWaitingTime:  //
		__asm("svc #0x03");
		break;
	}
}

static void MYRTOS_Create_MainStack()
{
	OS_Control._S_MSP = &_estack;
	OS_Control._E_MSP = OS_Control._S_MSP - MainStackSize;
	// Align 8 Bytes space between Main stack and Process stack
	OS_Control.PSP_Task_Locator = (OS_Control._E_MSP - 8);
}


static void MyRTOS_Idle_Task()
{
	while(1)
	{
		IdleTaskLED ^=1; //toggle LED
		__asm("wfe");    //wait for event to save power
	}
}

void SysTick_Handler(void)
{
	SysTickLED ^= 1;

	/**/
	MYRTOS_Update_TasksWaitingTime();

	/* Determine Current and Next tasks */
	MYRTOS_DecideNextTask();

	/* Switch context and restore */
	Trigger_OS_PendSV();
}

static void MYRTOS_DecideNextTask(void){
	/* If Ready Queue is empty && OS_Control->CurrentTask != Suspended */
	if(ReadyQueue.counter == 0 && OS_Control.CurrentTaskExecuted->TaskState != Suspended) {
	OS_Control.CurrentTaskExecuted->TaskState = Running;

	/* Add the current task again (Round Robin) */
	FIFO_Enqeue(&ReadyQueue, OS_Control.CurrentTaskExecuted);
	OS_Control.NextTaskTobeExecuted = OS_Control.CurrentTaskExecuted;
	}
	else{
		FIFO_Deqeue(&ReadyQueue, &OS_Control.NextTaskTobeExecuted);
		OS_Control.NextTaskTobeExecuted->TaskState = Running;

		/* Update Ready Queue (To keep round robin algorithm) */
		if((OS_Control.CurrentTaskExecuted->Priority == OS_Control.NextTaskTobeExecuted->Priority) && (OS_Control.CurrentTaskExecuted->TaskState != Suspended)){
			FIFO_Enqeue(&ReadyQueue, OS_Control.CurrentTaskExecuted);
			OS_Control.CurrentTaskExecuted->TaskState = Ready;
		}
	}
}

/* Used to execute specific OS Services */
void MYRTOS_SVC_services(uint32_t *StackFramePointer){
	/* OS_SVC_Set Stack -> r0 -> argument0 = StackFramePointer
	   OS_SVC_Set : r0,r1,r2,r3,r12,LR,PC,xPSR */
	uint8_t SVC_number;
	SVC_number = *((uint8_t*)((uint8_t*)(StackFramePointer[6])) - 2);
	switch(SVC_number){
	case SVC_ActivateTask:
	case SVC_TerminateTask:
	case SVC_TaskWaitingTime:
		/* Update Scheduler Table and Ready Queue */
		MYRTOS_UpdateSchedulerTable();

		/* If OS is in running state -> Decide what next task */
		if(OS_Control.OS_Modes_ID == OS_Running){
			if(strcmp(OS_Control.CurrentTaskExecuted->TaskName , "idletask") != 0)
			{
				MYRTOS_DecideNextTask();
				/* Switch/Restore Context */
				Trigger_OS_PendSV();
			}
		}
		break;
	}
}

__attribute ((naked)) void PendSV_Handler(void){
	/* Save context of current task */
	/* 1- Get the current task PSP */
	OS_GET_PSP(OS_Control.CurrentTaskExecuted->Current_PSP);

	/* 2- Use the PSP to store R4 to R11 */
	OS_Control.CurrentTaskExecuted->Current_PSP--;
	__asm volatile ("mov %0, r4" : "=r" (*(OS_Control.CurrentTaskExecuted->Current_PSP)));
	OS_Control.CurrentTaskExecuted->Current_PSP--;
	__asm volatile ("mov %0, r5" : "=r" (*(OS_Control.CurrentTaskExecuted->Current_PSP)));
	OS_Control.CurrentTaskExecuted->Current_PSP--;
	__asm volatile ("mov %0, r6" : "=r" (*(OS_Control.CurrentTaskExecuted->Current_PSP)));
	OS_Control.CurrentTaskExecuted->Current_PSP--;
	__asm volatile ("mov %0, r7" : "=r" (*(OS_Control.CurrentTaskExecuted->Current_PSP)));
	OS_Control.CurrentTaskExecuted->Current_PSP--;
	__asm volatile ("mov %0, r8" : "=r" (*(OS_Control.CurrentTaskExecuted->Current_PSP)));
	OS_Control.CurrentTaskExecuted->Current_PSP--;
	__asm volatile ("mov %0, r9" : "=r" (*(OS_Control.CurrentTaskExecuted->Current_PSP)));
	OS_Control.CurrentTaskExecuted->Current_PSP--;
	__asm volatile ("mov %0, r10" : "=r" (*(OS_Control.CurrentTaskExecuted->Current_PSP)));
	OS_Control.CurrentTaskExecuted->Current_PSP--;
	__asm volatile ("mov %0, r11" : "=r" (*(OS_Control.CurrentTaskExecuted->Current_PSP)));

	/* Restore context of next task */
	OS_Control.CurrentTaskExecuted = OS_Control.NextTaskTobeExecuted;
	OS_Control.NextTaskTobeExecuted = NULL;

	__asm volatile ("mov r11, %0" : : "r" (*(OS_Control.CurrentTaskExecuted->Current_PSP)));
	OS_Control.CurrentTaskExecuted->Current_PSP++;
	__asm volatile ("mov r10, %0" : : "r" (*(OS_Control.CurrentTaskExecuted->Current_PSP)));
	OS_Control.CurrentTaskExecuted->Current_PSP++;
	__asm volatile ("mov r9, %0" : : "r" (*(OS_Control.CurrentTaskExecuted->Current_PSP)));
	OS_Control.CurrentTaskExecuted->Current_PSP++;
	__asm volatile ("mov r8, %0" : : "r" (*(OS_Control.CurrentTaskExecuted->Current_PSP)));
	OS_Control.CurrentTaskExecuted->Current_PSP++;
	__asm volatile ("mov r7, %0" : : "r" (*(OS_Control.CurrentTaskExecuted->Current_PSP)));
	OS_Control.CurrentTaskExecuted->Current_PSP++;
	__asm volatile ("mov r6, %0" : : "r" (*(OS_Control.CurrentTaskExecuted->Current_PSP)));
	OS_Control.CurrentTaskExecuted->Current_PSP++;
	__asm volatile ("mov r5, %0" : : "r" (*(OS_Control.CurrentTaskExecuted->Current_PSP)));
	OS_Control.CurrentTaskExecuted->Current_PSP++;
	__asm volatile ("mov r4, %0" : : "r" (*(OS_Control.CurrentTaskExecuted->Current_PSP)));
	OS_Control.CurrentTaskExecuted->Current_PSP++;

	/* Update PSP and exit */
	OS_SET_PSP(OS_Control.CurrentTaskExecuted->Current_PSP);
	__asm volatile ("BX LR");
}

static void MYRTOS_UpdateSchedulerTable(void){
	Task_ref *temp = NULL;
	Task_ref *pTask, *pNextTask = NULL;
	uint8_t i = 0;

	/* Bubble sort Scheduler Table */
	MYRTOS_BubbleSort();

	/* Free Ready Queue */
	while(FIFO_Deqeue(&ReadyQueue, &temp) != FIFO_Empty);

	/* Update Ready Queue */
	while(i < OS_Control.ActiveTasksNum){
		pTask = OS_Control.OS_Tasks[i];
		pNextTask = OS_Control.OS_Tasks[i + 1];

		if(pTask->TaskState != Suspended){
			/* In case we reached to the end of available OSTasks */ /* Check for priority */
			if((pNextTask->TaskState == Suspended) || (pTask->Priority < pNextTask->Priority)){
				FIFO_Enqeue(&ReadyQueue, pTask);
				pTask->TaskState = Ready;
				break;
			}
			else if(pTask->Priority == pNextTask->Priority){
				/* If both tasks have the same priority
				 * Push pTask to ready state
				 * Make the pTask = pNextTask and pNextTask++
				 */
				FIFO_Enqeue(&ReadyQueue, pTask);
				pTask->TaskState = Ready;
			}
			else if(pTask->Priority > pNextTask->Priority){
				/* Not allowed to happen because we already did bubble sort */
				break;
			}
		}
		i++;
	}
}

static void MYRTOS_BubbleSort(void)
{
	uint8_t i, j, n;
	Task_ref *temp;
	n = OS_Control.ActiveTasksNum;
	for(i = 0; i < (n - 1); i++){
		for(j = 0; j < (n - i - 1); j++){
			if(OS_Control.OS_Tasks[j]->Priority > OS_Control.OS_Tasks[j+1]->Priority){
				temp = OS_Control.OS_Tasks[j];
				OS_Control.OS_Tasks[j] = OS_Control.OS_Tasks[j+1];
				OS_Control.OS_Tasks[j+1] = temp;
			}
		}
	}
}



MYRTOS_errorID MYRTOS_Init()
{
	MYRTOS_errorID ErrorState = noError;

	// Update OS Mode (Suspend)
	OS_Control.OS_Modes_ID = OS_Suspend;

	// Specify the Main stack for the OS
	MYRTOS_Create_MainStack();

	// Create OS Ready Queue
	if((FIFO_Init(&ReadyQueue, ReadyQueueFIFO, 100)) != FIFO_No_Error)
		ErrorState += readyQueueInitError;

	// Configure idle task
	if(ErrorState == noError)
	{
		strcpy(MYRTOS_IdleTask.TaskName,"idleTask");
		MYRTOS_IdleTask.Priority = 255;    // The least priority
		MYRTOS_IdleTask.pf_TaskEntry = MyRTOS_Idle_Task;
		MYRTOS_IdleTask.Stack_Size = 300;  // Bytes
		ErrorState += MYRTOS_Create_Task(&MYRTOS_IdleTask);
	}
	return ErrorState;
}


static void MYRTOS_Create_Stack(Task_ref* taskReference)
{
	/*
	 * Task Frame
	 * ==========
	 * CPU Saved registers
	 *
	 * xPSR
	 * PC (Next instruction to be executed in the task)
	 * LR (return register which is saved in CPU before switching)
	 * r12
	 * r3
	 * r2
	 * r1
	 * r0
	 * ==========
	 * Manually Saved/Restored registers
	 *
	 * r4
	 * r5
	 * r6
	 * r7
	 * r8
	 * r9
	 * r10
	 * r11
	 */
	uint8_t index;
	taskReference->Current_PSP = (uint32_t*)taskReference->_S_PSP_Task;

	taskReference->Current_PSP--;
	*(taskReference->Current_PSP) = 0x01000000;  // Dummy psr (by default T=1 Thumb2) to avoid bus fault

	taskReference->Current_PSP--;
	*(taskReference->Current_PSP) = (unsigned int)(taskReference->pf_TaskEntry);   // PC value

	taskReference->Current_PSP--;
	*(taskReference->Current_PSP) = (unsigned int)(0xFFFFFFFD);   // (Dummy value)As if LR (This function came from interrupt

	// The rest is 13 registers we want to push with 0 for now
	for(index = 0; index < 13; index++)
	{
		taskReference->Current_PSP--;
		*(taskReference->Current_PSP) = 0;
	}
}



MYRTOS_errorID MYRTOS_Create_Task(Task_ref* taskReference)
{
	MYRTOS_errorID ErrorState = noError;

	// Create its own PS Stack
	taskReference->_S_PSP_Task = OS_Control.PSP_Task_Locator;
	taskReference->_E_PSP_Task = taskReference->_S_PSP_Task -  taskReference->Stack_Size;

	// Check that task size isn't exceeding Process stack size
	if(taskReference->_E_PSP_Task < (uint32_t)(&_eheap))
		return taskExceededStackSize;

	// Align 8 Byte for the next task PS stack
	OS_Control.PSP_Task_Locator = taskReference->_E_PSP_Task - 8;

	// - _S_PSP_Task
	// - Task stack
	// - _E_PSP_Task

	// - _eheap


	// Initialize PS Task Stack
	MYRTOS_Create_Stack(taskReference);

	OS_Control.OS_Tasks[OS_Control.ActiveTasksNum] = taskReference;
	OS_Control.ActiveTasksNum++;

	if(taskReference->AutoStart == Autostart_Enabled)
	{
		taskReference->TaskState = Running;
	}
	else
	{
		taskReference->TaskState = Suspended;
	}
	return ErrorState;

}

static void MYRTOS_Update_TasksWaitingTime(void){
	uint8_t index;
	for(index = 0; index < OS_Control.ActiveTasksNum; index++){
		if(OS_Control.OS_Tasks[index]->TaskState == Suspended){
			if(OS_Control.OS_Tasks[index]->TimeWaiting.Task_Block_State == enabled){
				OS_Control.OS_Tasks[index]->TimeWaiting.Ticks_Count--;
				if(OS_Control.OS_Tasks[index]->TimeWaiting.Ticks_Count == 0){
					OS_Control.OS_Tasks[index]->TimeWaiting.Task_Block_State = disabled;
					OS_Control.OS_Tasks[index]->TaskState = Waiting;
					OS_SVC_Set(SVC_TaskWaitingTime);
				}
			}
		}
	}
}


void MYRTOS_StartOS(void){
	OS_Control.OS_Modes_ID = OS_Running;
	/* Set default "Current Task" */
	OS_Control.CurrentTaskExecuted = &MyRTOS_Idle_Task;

	MYRTOS_ActivateTask(&MyRTOS_Idle_Task);

	/* Start Ticker */
	Start_Ticker();

	/* Set PSP */
	OS_SET_PSP(OS_Control.CurrentTaskExecuted->Current_PSP);
	OS_SWITCH_SP_to_PSP();

	/* Switch to thread mode and unprivileged */
	OS_SWITCH_TO_UNPRIVILIGE;
	OS_Control.CurrentTaskExecuted->pf_TaskEntry();
}


MYRTOS_errorID MYRTOS_AcquireMutex(Mutex_ref* pMutex, Task_ref* pTask){
	MYRTOS_errorID retval = noError;

	if(pMutex->CurrentTUser == NULL)
	{
		pMutex->CurrentTUser = pTask;
		if(pMutex->PriorityCeiling.state == PriorityCeiling_enabled)
		{
			/* Save current task priority */
			pMutex->PriorityCeiling.old_priority = pMutex->CurrentTUser->Priority;

			/* Change task priority to ceiling priority */
			pMutex->CurrentTUser->Priority = pMutex->PriorityCeiling.Ceiling_Priority;
		}
	}
	else if(pMutex->CurrentTUser == pTask)
	{
		retval = MutexIsAlreadyAcquired;
	}
	else if(pMutex->NextTUser == NULL)
	{
		pMutex->NextTUser = pTask;
		pMutex->NextTUser->TaskState = Suspended;
		OS_SVC_Set(SVC_TerminateTask);
	}
	else
	{
		retval = MutexReachedMaxNoOfUsers;
	}

	return retval;
}


void MYRTOS_ReleaseMutex(Mutex_ref* pMutex){
	if((pMutex->CurrentTUser != NULL) && (pMutex->CurrentTUser == OS_Control.CurrentTaskExecuted)){
		if(pMutex->PriorityCeiling.state == PriorityCeiling_enabled){
		/* Restore current task priority */
		pMutex->CurrentTUser->Priority = pMutex->PriorityCeiling.old_priority;
		}

		if(pMutex->NextTUser == NULL){
			pMutex->CurrentTUser = NULL;
		}
		else{
			pMutex->CurrentTUser = pMutex->NextTUser;
			pMutex->NextTUser = NULL;
			pMutex->CurrentTUser->TaskState = Waiting;
			if(pMutex->PriorityCeiling.state == PriorityCeiling_enabled){
				/* Save current task priority */
				pMutex->PriorityCeiling.old_priority = pMutex->CurrentTUser->Priority;

				/* Change task priority to ceiling priority */
				pMutex->CurrentTUser->Priority = pMutex->PriorityCeiling.Ceiling_Priority;
			}
			OS_SVC_Set(SVC_ActivateTask);
		}
	}
}

void MYRTOS_ActivateTask(Task_ref* pTask){
	/* Change Task State */
	pTask->TaskState = Waiting;

	OS_SVC_Set(SVC_ActivateTask);
}

void MYRTOS_TerminateTask(Task_ref* pTask){
	/* Change Task State */
	pTask->TaskState = Suspended;

	OS_SVC_Set(SVC_TerminateTask);
}

void MYRTOS_TaskWait(uint32_t Ticks, Task_ref* pTask){
	pTask->TimeWaiting.Task_Block_State = enabled;
	pTask->TimeWaiting.Ticks_Count = Ticks;

	/* Task should be blocked */
	pTask->TaskState = Suspended;
	OS_SVC_Set(SVC_TerminateTask);
}

