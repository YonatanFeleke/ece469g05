################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../os/clock.c \
../os/filesys.c \
../os/memory.c \
../os/misc.c \
../os/process.c \
../os/queue.c \
../os/shared_memory.c \
../os/synch.c \
../os/sysproc.c \
../os/traps.c 

OBJS += \
./os/clock.o \
./os/filesys.o \
./os/memory.o \
./os/misc.o \
./os/process.o \
./os/queue.o \
./os/shared_memory.o \
./os/synch.o \
./os/sysproc.o \
./os/traps.o 

C_DEPS += \
./os/clock.d \
./os/filesys.d \
./os/memory.d \
./os/misc.d \
./os/process.d \
./os/queue.d \
./os/shared_memory.d \
./os/synch.d \
./os/sysproc.d \
./os/traps.d 


# Each subdirectory must supply rules for building sources it contributes
os/%.o: ../os/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


