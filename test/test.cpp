#include "../src/socket/socket.h"
#include <stdio.h>

int main() {
	socket_InitializeSockets();
	SOCKETH_T iSocket = socket_CreateSocket(NULL, NULL, NULL);
	socket_BindSocket(iSocket, "192.168.1.106", 80, NULL);
	socket_Listen(iSocket, NULL);
	printf("Wait for connection...\n");
	SOCKETH_T connSocket = socket_Accept(iSocket);
	char buffer[2048];
	int bRecv = socket_Recv(connSocket, buffer, 2048, NULL);
	buffer[bRecv] = '\0';
	printf("Received:\n\n=====STR=====\n%s\n=====END=====\n", buffer);
	memset(buffer, NULL, 2048); // Clear buffer
	socket_Send(iSocket, buffer, NULL, NULL); // Close the connection
	socket_Close(connSocket);
	socket_Close(iSocket);
	socket_ShutdownSockets();
	return 0;
}