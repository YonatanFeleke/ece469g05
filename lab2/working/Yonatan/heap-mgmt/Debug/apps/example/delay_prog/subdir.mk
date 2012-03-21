################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../apps/example/delay_prog/delay_prog.c 

OBJS += \
./apps/example/delay_prog/delay_prog.o 

C_DEPS += \
./apps/example/delay_prog/delay_prog.d 


# Each subdirectory must supply rules for building sources it contributes
apps/example/delay_prog/%.o: ../apps/example/delay_prog/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


