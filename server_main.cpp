#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

int main(int ac, char** av) 
{
	// Initialize winsock.
	WSADATA ws_data;
	if (WSAStartup(MAKEWORD(2, 2), &ws_data) != 0) 
	{
		printf("cannot initialize winsock!\n");
		return -1;
	}

	// Create a listening socket.
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		printf("Can't create socket!\n");
		return -2;
	}

	// Bind the ip to the socket.
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(42000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(listening, (sockaddr*)& hint, sizeof(hint)) == SOCKET_ERROR)
	{
		printf("Could not bind to address\n");
		return -3;
	}

	
	// TODO here come the code!


	// Close winsock.
	WSACleanup();
	return 0;
}