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
-include apps/work/subdir.mk
-include apps/example/userprog/work/subdir.mk
-include apps/example/userprog/subdir.mk
-include apps/example/stack_grow/work/subdir.mk
-include apps/example/stack_grow/subdir.mk
-include apps/example/makesimultprocs/work/subdir.mk
-include apps/example/makesimultprocs/subdir.mk
-include apps/example/makeprocs/work/subdir.mk
-include apps/example/makeprocs/subdir.mk
-include apps/example/invalid_page/work/subdir.mk
-include apps/example/invalid_page/subdir.mk
-include apps/example/invalid_address/work/subdir.mk
-include apps/example/invalid_address/subdir.mk
-include apps/example/hello_world/work/subdir.mk
-include apps/example/hello_world/subdir.mk
-include apps/example/delay_prog/work/subdir.mk
-include apps/example/delay_prog/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: one-level

# Tool invocations
one-level: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o"one-level" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) one-level
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
