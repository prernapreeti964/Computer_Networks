MultiThreaded TCP Web Server and Web Client:

To run the Connection Oriented Web Server and Web Client:

1. Compile server_TCP.c file in the 

CN_assignment1/TCP_server/ using the command: 

make all

2. Run the TCP Server: 

./TCP_server <port>

3. Compile TCP Client in the 

CN_assignment1/TCP_client/client.c

make all

4. Run the Client: 

For persistent connections: 

./TCPclient <server_hostname> <port> <connection_type(p/np)> <filelist>

For non-persistent connections: 

./TCPclient <server_hostname> <port> <connection_type(p/np)> <file>

UDP  Web Server and Web Client:

To run the UDP Web Server and Web Client:

1. Compile UDPserver in the 

CN_assignment1/UDPserver/server.cpp using the command: 

make all

2. Run the UDPserver: 

./UDP_server <port>

3. Compile UDPclient in the 

CN_assignment1/UDPclient/client_UDP.cpp

make all

4. Run the UDPClient:

./client_UDP <server_IP/hostname> <server_port> <file>

Note: To clean executable files just run : make clean
