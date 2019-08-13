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

	// TODO here come the code!

	// Close winsock.
	WSACleanup();
	return 0;
}