################################################################################
# make file for TCP server
################################################################################

-include ../makefile.init

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: TCP_server

# Tool invocations
TCP_server: 
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	gcc  -pthread -o TCP_server server_TCP.c
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	rm -f TCP_server

-include ../makefile.targets
