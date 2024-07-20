################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Float_transform/Float_transform.c 

OBJS += \
./Core/Src/Float_transform/Float_transform.o 

C_DEPS += \
./Core/Src/Float_transform/Float_transform.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Float_transform/%.o Core/Src/Float_transform/%.su Core/Src/Float_transform/%.cyclo: ../Core/Src/Float_transform/%.c Core/Src/Float_transform/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F303xE -c -I../Core/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Float_transform

clean-Core-2f-Src-2f-Float_transform:
	-$(RM) ./Core/Src/Float_transform/Float_transform.cyclo ./Core/Src/Float_transform/Float_transform.d ./Core/Src/Float_transform/Float_transform.o ./Core/Src/Float_transform/Float_transform.su

.PHONY: clean-Core-2f-Src-2f-Float_transform

