################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../apps/example/excess_pages/excess_pages.c 

OBJS += \
./apps/example/excess_pages/excess_pages.o 

C_DEPS += \
./apps/example/excess_pages/excess_pages.d 


# Each subdirectory must supply rules for building sources it contributes
apps/example/excess_pages/%.o: ../apps/example/excess_pages/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


