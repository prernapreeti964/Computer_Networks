################################################################################
# makefile for UDP client
################################################################################

-include ../makefile.init

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: UDP_client

# Tool invocations
UDP_client: 
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	g++ -o UDPclient client_UDP.cpp
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	rm -f UDPclient

-include ../makefile.targets
