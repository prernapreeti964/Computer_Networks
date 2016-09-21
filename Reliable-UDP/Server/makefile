################################################################################
# makefile for RUDP server
################################################################################

-include ../makefile.init

# All Target
all: RUDPserver

# Tool invocations
RUDPserver: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross G++ Linker'
	g++  -o RUDPserver server.cpp
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	rm -f RUDPserver
-include ../makefile.targets
