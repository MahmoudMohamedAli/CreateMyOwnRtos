################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../My_RTOS/Cortex_Mx_OS_Porting.c \
../My_RTOS/My_RTOS_FIFO.c \
../My_RTOS/Schedular.c 

OBJS += \
./My_RTOS/Cortex_Mx_OS_Porting.o \
./My_RTOS/My_RTOS_FIFO.o \
./My_RTOS/Schedular.o 

C_DEPS += \
./My_RTOS/Cortex_Mx_OS_Porting.d \
./My_RTOS/My_RTOS_FIFO.d \
./My_RTOS/Schedular.d 


# Each subdirectory must supply rules for building sources it contributes
My_RTOS/%.o My_RTOS/%.su My_RTOS/%.cyclo: ../My_RTOS/%.c My_RTOS/subdir.mk
	arm-none-eabi-gcc -gdwarf-2 "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -c -I../Inc -I"D:/STM32_WorkSpace/ARM_CortexM4_M3/CMSIS_V5" -I"D:/STM32_WorkSpace/ARM_CortexM4_M3/My_RTOS/inc" -I"D:/STM32_WorkSpace/ARM_CortexM4_M3/Stm32_F103C6_Drivers/inc" -I"D:/STM32_WorkSpace/ARM_CortexM4_M3/HAL/includes" -I"D:/STM32_WorkSpace/ARM_CortexM4_M3/Stm32_F103C6_Drivers" -I"D:/STM32_WorkSpace/ARM_CortexM4_M3/HAL" -I"D:/STM32_WorkSpace/ARM_CortexM4_M3/My_RTOS" -I"D:/STM32_WorkSpace/ARM_CortexM4_M3/My_RTOS/inc" -I"D:/STM32_WorkSpace/ARM_CortexM4_M3/Src" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-My_RTOS

clean-My_RTOS:
	-$(RM) ./My_RTOS/Cortex_Mx_OS_Porting.cyclo ./My_RTOS/Cortex_Mx_OS_Porting.d ./My_RTOS/Cortex_Mx_OS_Porting.o ./My_RTOS/Cortex_Mx_OS_Porting.su ./My_RTOS/My_RTOS_FIFO.cyclo ./My_RTOS/My_RTOS_FIFO.d ./My_RTOS/My_RTOS_FIFO.o ./My_RTOS/My_RTOS_FIFO.su ./My_RTOS/Schedular.cyclo ./My_RTOS/Schedular.d ./My_RTOS/Schedular.o ./My_RTOS/Schedular.su

.PHONY: clean-My_RTOS

