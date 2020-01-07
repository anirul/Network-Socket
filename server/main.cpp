#if defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include <stdio.h>
#include <vector>
#include <list>
#if defined(_WIN32) || defined(_WIN64)
#pragma comment(lib, "ws2_32.lib")
#else
#define SOCKET int
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#endif

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
		printf("Send %d elements.\n", (int)buffer.size());
		if (result == 0)
		{
			result = -1;
		}
	}
	return (result >= 0);
}

int main(int ac, char** av)
{
#if defined(_WIN32) || defined(WIN64)
	// Initialize winsock.
	WSADATA ws_data;
	if (WSAStartup(MAKEWORD(2, 2), &ws_data) != 0)
	{
		printf("cannot initialize winsock!\n");
		return -1;
	}
#endif

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
#if defined(_WIN32) || defined(_WIN64)
	hint.sin_addr.S_un.S_addr = INADDR_ANY;
#else
	hint.sin_addr.s_addr = INADDR_ANY;
#endif
	if (bind(listening, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR)
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
			printf("New client.\n");
		}
		for (auto client : connection_vec)
		{
			std::vector<char> buffer;
			if (!check_recv(client, buffer))
			{
				connection_vec.remove_if([client](SOCKET sock) {
					return sock == client;
				});
#if defined(_WIN32) || defined(_WIN64)
				closesocket(client);
#else
				close(client);
#endif
				printf("Client disconnected on recv.\n");
				break;
			}
			if (buffer.size() != 0)
			{
				if (!check_send(client, buffer))
				{
					connection_vec.remove_if([client](SOCKET sock) {
						return sock == client;
					});
#if defined(_WIN32) || defined(_WIN64)
					closesocket(client);
#else
					close(client);
#endif
					printf("Client disconnected on send.\n");
					break;
				}
			}
		}
#if defined(_WIN32) || defined(_WIN64)
		Sleep(1);
#else
		usleep(1);
#endif
	}

#if defined(_WIN32) || defined(_WIN64)
	closesocket(listening);
	WSACleanup();
#else
	close(listening);
#endif
	return 0;
}
