################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include os/work/subdir.mk
-include os/subdir.mk
-include lib/subdir.mk
-include bin/subdir.mk
-include apps/ostests/ostests/subdir.mk
-include apps/fdisk/fdisk/work/subdir.mk
-include apps/fdisk/fdisk/subdir.mk
-include apps/example/spawn_me/subdir.mk
-include apps/example/makeprocs/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: flat

# Tool invocations
flat: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o"flat" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) flat
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
