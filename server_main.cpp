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
		printf("Could not bind to address.\n");
		return -3;
	}

	// Listen to the socket.
	if (listen(listening, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("Could not listen to socket.\n");
		return -4;
	}
	
	// Accept socket and accept FIXME multithread/fork/etc...?
	printf("Wait for connection...\n");
	SOCKET client = accept(listening, nullptr, nullptr);
	if (client == INVALID_SOCKET)
	{
		printf("accept failed.\n");
		return -5;
	}
	printf("Client connected!\n");

	const size_t DEFAULT_BUFLEN = 512;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult, iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// Receive until the peer shuts down the connection
	do {

		iResult = recv(client, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);

			// Echo the buffer back to the sender
			iSendResult = send(client, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed: %d\n", WSAGetLastError());
				closesocket(client);
				WSACleanup();
				return 1;
			}
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(client);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);

	// Close sockets
	closesocket(listening);
	// Close winsock.
	WSACleanup();
	return 0;
}