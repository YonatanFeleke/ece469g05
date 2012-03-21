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
-include apps/example/stack_grow/work/subdir.mk
-include apps/example/stack_grow/subdir.mk
-include apps/example/shared_test/work/subdir.mk
-include apps/example/shared_test/subdir.mk
-include apps/example/makeprocs/work/subdir.mk
-include apps/example/makeprocs/subdir.mk
-include apps/example/invalid_page/work/subdir.mk
-include apps/example/invalid_page/subdir.mk
-include apps/example/invalid_address/work/subdir.mk
-include apps/example/invalid_address/subdir.mk
-include apps/example/hello_world/work/subdir.mk
-include apps/example/hello_world/subdir.mk
-include apps/example/excess_pages/work/subdir.mk
-include apps/example/excess_pages/subdir.mk
-include apps/example/delay_prog/work/subdir.mk
-include apps/example/delay_prog/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: shared-mem

# Tool invocations
shared-mem: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++  -o"shared-mem" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C++_DEPS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) shared-mem
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
