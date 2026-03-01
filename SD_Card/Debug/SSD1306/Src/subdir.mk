################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SSD1306/Src/fonts.c \
../SSD1306/Src/ssd1306.c 

OBJS += \
./SSD1306/Src/fonts.o \
./SSD1306/Src/ssd1306.o 

C_DEPS += \
./SSD1306/Src/fonts.d \
./SSD1306/Src/ssd1306.d 


# Each subdirectory must supply rules for building sources it contributes
SSD1306/Src/%.o SSD1306/Src/%.su SSD1306/Src/%.cyclo: ../SSD1306/Src/%.c SSD1306/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-SSD1306-2f-Src

clean-SSD1306-2f-Src:
	-$(RM) ./SSD1306/Src/fonts.cyclo ./SSD1306/Src/fonts.d ./SSD1306/Src/fonts.o ./SSD1306/Src/fonts.su ./SSD1306/Src/ssd1306.cyclo ./SSD1306/Src/ssd1306.d ./SSD1306/Src/ssd1306.o ./SSD1306/Src/ssd1306.su

.PHONY: clean-SSD1306-2f-Src

