################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../apps/example/makeprocs/makeprocs.c 

OBJS += \
./apps/example/makeprocs/makeprocs.o 

C_DEPS += \
./apps/example/makeprocs/makeprocs.d 


# Each subdirectory must supply rules for building sources it contributes
apps/example/makeprocs/%.o: ../apps/example/makeprocs/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


