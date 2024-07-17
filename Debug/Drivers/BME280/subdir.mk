################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BME280/BME280.c 

OBJS += \
./Drivers/BME280/BME280.o 

C_DEPS += \
./Drivers/BME280/BME280.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BME280/%.o Drivers/BME280/%.su Drivers/BME280/%.cyclo: ../Drivers/BME280/%.c Drivers/BME280/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F303xE -c -I../Core/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BME280

clean-Drivers-2f-BME280:
	-$(RM) ./Drivers/BME280/BME280.cyclo ./Drivers/BME280/BME280.d ./Drivers/BME280/BME280.o ./Drivers/BME280/BME280.su

.PHONY: clean-Drivers-2f-BME280

