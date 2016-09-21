RUDP  Web Server and Web Client:

To run the RUDP  Web Server and Web Client:

1. Compile RUDPserver in the 

RUDP/Server/server.cpp using the command: 

make all

2. Run the RUDPserver: 

./RUDPserver <port> <window size in bytes>

3. Compile RUDPclient in the 

RUDP/Client/client_RUDP.cpp

make all

4. Run the UDPClient:

./RUDPclient  <server_IP/hostname> <server_port> <file> <window size in bytes>

Note: To clean executable files just run : make clean
