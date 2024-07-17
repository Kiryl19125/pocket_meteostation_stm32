################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/ds3231/ds3231_for_stm32_hal.c 

OBJS += \
./Drivers/ds3231/ds3231_for_stm32_hal.o 

C_DEPS += \
./Drivers/ds3231/ds3231_for_stm32_hal.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/ds3231/%.o Drivers/ds3231/%.su Drivers/ds3231/%.cyclo: ../Drivers/ds3231/%.c Drivers/ds3231/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F303xE -c -I../Core/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-ds3231

clean-Drivers-2f-ds3231:
	-$(RM) ./Drivers/ds3231/ds3231_for_stm32_hal.cyclo ./Drivers/ds3231/ds3231_for_stm32_hal.d ./Drivers/ds3231/ds3231_for_stm32_hal.o ./Drivers/ds3231/ds3231_for_stm32_hal.su

.PHONY: clean-Drivers-2f-ds3231

