################################################################################
# makefile for TCP client
################################################################################

-include ../makefile.init

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: TCP_client

# Tool invocations
TCP_client: 
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	gcc  -o TCPclient client.c
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	rm -f TCPclient

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
