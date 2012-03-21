################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../apps/example/test2/test2.c 

OBJS += \
./apps/example/test2/test2.o 

C_DEPS += \
./apps/example/test2/test2.d 


# Each subdirectory must supply rules for building sources it contributes
apps/example/test2/%.o: ../apps/example/test2/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


