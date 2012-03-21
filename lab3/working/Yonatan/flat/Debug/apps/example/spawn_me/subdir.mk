################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../apps/example/spawn_me/spawn_me.c 

OBJS += \
./apps/example/spawn_me/spawn_me.o 

C_DEPS += \
./apps/example/spawn_me/spawn_me.d 


# Each subdirectory must supply rules for building sources it contributes
apps/example/spawn_me/%.o: ../apps/example/spawn_me/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


