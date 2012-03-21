################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../apps/ostests/ostests/ostests.c 

OBJS += \
./apps/ostests/ostests/ostests.o 

C_DEPS += \
./apps/ostests/ostests/ostests.d 


# Each subdirectory must supply rules for building sources it contributes
apps/ostests/ostests/%.o: ../apps/ostests/ostests/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


