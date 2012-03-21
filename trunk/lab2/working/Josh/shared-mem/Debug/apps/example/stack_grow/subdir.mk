################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../apps/example/stack_grow/stack_grow.c 

OBJS += \
./apps/example/stack_grow/stack_grow.o 

C_DEPS += \
./apps/example/stack_grow/stack_grow.d 


# Each subdirectory must supply rules for building sources it contributes
apps/example/stack_grow/%.o: ../apps/example/stack_grow/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


