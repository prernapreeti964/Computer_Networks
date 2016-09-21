################################################################################
# make file for UDP server
################################################################################

-include ../makefile.init

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: UDP_server

# Tool invocations
UDP_server: 
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	g++ -o UDP_server server.cpp
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	rm -f UDP_server

-include ../makefile.targets
