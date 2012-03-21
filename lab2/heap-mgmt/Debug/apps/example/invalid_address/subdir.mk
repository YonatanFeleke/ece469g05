################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../apps/example/invalid_address/invalid_address.c 

OBJS += \
./apps/example/invalid_address/invalid_address.o 

C_DEPS += \
./apps/example/invalid_address/invalid_address.d 


# Each subdirectory must supply rules for building sources it contributes
apps/example/invalid_address/%.o: ../apps/example/invalid_address/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


