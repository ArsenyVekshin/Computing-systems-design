################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/lab1/button.c \
../Core/Src/lab1/lab1.c \
../Core/Src/lab1/led.c 

OBJS += \
./Core/Src/lab1/button.o \
./Core/Src/lab1/lab1.o \
./Core/Src/lab1/led.o 

C_DEPS += \
./Core/Src/lab1/button.d \
./Core/Src/lab1/lab1.d \
./Core/Src/lab1/led.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/lab1/%.o Core/Src/lab1/%.su: ../Core/Src/lab1/%.c Core/Src/lab1/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F427xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-lab1

clean-Core-2f-Src-2f-lab1:
	-$(RM) ./Core/Src/lab1/button.d ./Core/Src/lab1/button.o ./Core/Src/lab1/button.su ./Core/Src/lab1/lab1.d ./Core/Src/lab1/lab1.o ./Core/Src/lab1/lab1.su ./Core/Src/lab1/led.d ./Core/Src/lab1/led.o ./Core/Src/lab1/led.su

.PHONY: clean-Core-2f-Src-2f-lab1

