################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../apps/example/heap_test/heap_test.c 

OBJS += \
./apps/example/heap_test/heap_test.o 

C_DEPS += \
./apps/example/heap_test/heap_test.d 


# Each subdirectory must supply rules for building sources it contributes
apps/example/heap_test/%.o: ../apps/example/heap_test/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


