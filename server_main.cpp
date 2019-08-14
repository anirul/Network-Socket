#include <WinSock2.h>
#include <stdio.h>
#include <vector>
#include <list>

#pragma comment(lib, "ws2_32.lib")

bool select_recv(SOCKET sock, int interval_us = 1)
{
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sock, &fds);
	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = interval_us;
	return (select(sock + 1, &fds, 0, 0, &tv) == 1);
}

bool check_accept(SOCKET listening, SOCKET* client_ptr)
{
	SOCKET client = INVALID_SOCKET;
	if (select_recv(listening))
	{
		// accept incoming connections.
		client = accept(listening, nullptr, nullptr);
	}
	*client_ptr = client;
	return (client != INVALID_SOCKET);
}

bool check_recv(SOCKET client, std::vector<char>& buffer)
{
	int result = 0;
	if (select_recv(client))
	{
		const int MAX_SIZE = 512;
		buffer.resize(MAX_SIZE, 0);
		result = recv(client, &buffer[0], MAX_SIZE, 0);
		buffer.resize(result);
		printf("Receive %d elements.\n", result);
		if (result == 0) 
		{
			result = -1;
		}
	}
	return (result >= 0);
}

bool check_send(SOCKET client, const std::vector<char>& buffer)
{
	int result = 0;
	if (buffer.size() > 0)
	{
		result = send(client, &buffer[0], buffer.size(), 0);
		printf("Send %d elements.\n", buffer.size());
	}
	return (result >= 0);
}

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
	
	printf("Wait for connection...\n");
	std::list<SOCKET> connection_vec;
	// start of the main loop.
	while (true)
	{
		SOCKET new_client;
		if (check_accept(listening, &new_client))
		{
			connection_vec.push_back(new_client);
			printf("New client.");
		}
		for (auto client : connection_vec)
		{
			std::vector<char> buffer;
			if (!check_recv(client, buffer))
			{
				connection_vec.remove_if([client](SOCKET sock) {
					return sock == client;
				});
				printf("Client disconnected on recv.");
				break;
			}
			if (buffer.size() != 0)
			{
				if (!check_send(client, buffer))
				{
					connection_vec.remove_if([client](SOCKET sock) {
						return sock == client;
					});
					printf("Client disconnected on send.");
					break;
				}
			}
		}
		Sleep(1);
	}

	// Close sockets
	closesocket(listening);
	// Close winsock.
	WSACleanup();
	return 0;
}