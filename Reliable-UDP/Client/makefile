################################################################################
# makefile for RUDP client
################################################################################

-include ../makefile.init

# All Target
all: RUDPclient

# Tool invocations
RUDPclient: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross G++ Linker'
	g++  -o RUDPclient client_RUDP.cpp
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	rm -f RUDPclient
-include ../makefile.targets
