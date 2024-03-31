/*
 * Schedular.h
 *
 *  Created on: Mar 16, 2024
 *      Author: Mahmoud Elkot
 */

#ifndef INC_SCHEDULAR_H_
#define INC_SCHEDULAR_H_

#include "Cortex_Mx_OS_Porting.h"

//----------------------------------------------
// Section: Macros Configuration References
//----------------------------------------------
#define MAX_NO_TASKS	100


typedef enum
{
	noError,
	readyQueueInitError,
	taskExceededStackSize,
	MutexReachedMaxNoOfUsers,
	MutexIsAlreadyAcquired
}MYRTOS_errorID;

typedef struct{
	const char TaskName[30];
	enum{
		Autostart_Disabled,
		Autostart_Enabled
	}AutoStart;
	uint8_t Priority;
	void (*pf_TaskEntry)(void); /* Pointer to Task C Function*/

	uint32_t Stack_Size;
	uint32_t _S_PSP_Task;	 /* Not entered by the user */
	uint32_t _E_PSP_Task;	 /* Not entered by the user */
	uint32_t* Current_PSP; /* Not entered by the user */

	enum{
		Suspended,
		Running,
		Waiting,
		Ready
	}TaskState;

	struct{
		enum{
			enabled,
			disabled
		}Task_Block_State;
		uint32_t Ticks_Count;
	}TimeWaiting;
}Task_ref;


typedef struct{
	uint8_t *pPayload;
	uint32_t PayloadSize;
	Task_ref* CurrentTUser;
	Task_ref* NextTUser;
	char MutexName[30];
	struct{
		enum{
			PriorityCeiling_enabled,
			PriorityCeiling_disabled
		}state;
		uint8_t Ceiling_Priority;
		uint8_t old_priority;
	}PriorityCeiling;
}Mutex_ref;

/**=============================================
 * @Fn			- MYRTOS_Init
 * @brief 		- Initializes the OS control and buffers
 * @retval 		- Returns noError if no error happened or an error code if an error occured
 * Note			- Must be called before using any of the OS APIs
 */
MYRTOS_errorID MYRTOS_Init(void);

/**=============================================
 * @Fn			- MYRTOS_CreateTask
 * @brief 		- Creates the task object in the OS and initializes the task's stack area
 * @param [in] 	- newTask: Pointer to the task's configuration
 * @retval 		- Returns noError if no error happened or an error code if an error occured
 * Note			- Should only be called after calling "MYRTOS_Init"
 */
MYRTOS_errorID MYRTOS_Create_Task(Task_ref* newTask);

/**=============================================
 * @Fn			- MYRTOS_ActivateTask
 * @brief 		- Sends a task to the ready queue to be scheduled
 * @param [in] 	- pTask: Pointer to the task's configuration
 * @retval 		- None
 * Note			- Should only be called after calling "MYRTOS_CreateTask"
 */
void MYRTOS_ActivateTask(Task_ref* pTask);

/**=============================================
 * @Fn			- MYRTOS_TerminateTask
 * @brief 		- Sends a task to the suspended state
 * @param [in] 	- pTask: Pointer to the task's configuration
 * @retval 		- None
 * Note			- Should only be called after calling "MYRTOS_CreateTask"
 */
void MYRTOS_TerminateTask(Task_ref* pTask);

/**=============================================
 * @Fn			- MYRTOS_StartOS
 * @brief 		- Starts the OS scheduler to begin running tasks
 * @retval 		- None
 * Note			- Should only be called after calling "MYRTOS_Init" and creating & activating tasks
 */
void MYRTOS_StartOS(void);

/**=============================================
 * @Fn			- MYRTOS_TaskWait
 * @brief 		- Sends a tasks to the waiting state for a specific amount of Ticks
 * @param [in] 	- Ticks: The amount of ticks the task should be suspended before running again
 * @param [in] 	- pTask: Pointer to the task's configuration
 * @retval 		- None.
 */
void MYRTOS_TaskWait(uint32_t Ticks, Task_ref* pTask);

/**=============================================
 * @Fn			- MYRTOS_AcquireMutex
 * @brief 		- Tries to acquire a mutex if available
 * @param [in] 	- pMutex: Pointer to the Mutex to be locked
 * @param [in] 	- pTask: Pointer to the task's configuration
 * @retval 		- Returns noError if no error happened or an error code if an error occured
 * Note			- A Mutex can be hold by 2 tasks only at the same time (as in a queue)
 */
MYRTOS_errorID MYRTOS_AcquireMutex(Mutex_ref* pMutex, Task_ref* pTask);

/**=============================================
 * @Fn			- MYRTOS_ReleaseMutex
 * @brief 		- Releases a mutex and starts the next task that is in the queue (if found)
 * @param [in] 	- pMutex: Pointer to the Mutex to be locked
 * @retval 		- None
 * Note			- A mutex can only be released by the same task that acquired it
 */
void MYRTOS_ReleaseMutex(Mutex_ref* pMutex);

#endif /* INC_SCHEDULAR_H_ */
