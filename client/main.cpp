#if defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <stdio.h>
#include <cstdlib>
#include <vector>
#include <iostream>
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

int main(int ac, char** av) {
	if (ac != 3)
	{
		std::cerr << "error incorrect argument list:" << std::endl;
		std::cerr << "program_name client port" << std::endl;
		return -1;
	}
#if defined(_WIN32) || defined(WIN64)
	WSADATA ws_data;
	if (WSAStartup(MAKEWORD(2, 2), &ws_data) != 0)
	{
		printf("cannot initialize winsock!\n");
		return -1;
	}
#endif
	SOCKET client = INVALID_SOCKET;
	if ((client = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		std::cerr << "could not create socket!" << std::endl;
		return -1;
	}
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(std::atoi(av[2]));
	if (inet_pton(AF_INET, av[1], &server_addr.sin_addr))
	{
		std::cerr << "invalid address " << av[1] << std::endl;
		return -1;
	}
	if (connect(client, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		std::cerr << "connection error" << std::endl;
		return -1;
	}
	std::cout << "Connected to " << av[1] << ":" << av[2] << std::endl;
	std::cout << "\tdisconnect by typing /quit" << std::endl;
	bool quit = false;
	while (!quit)
	{
		std::cout << "> ";
		std::string input;
		std::cin >> input;
		if (input.find("/quit") != std::string::npos)
		{
			quit = true;
			continue;
		}
		if (input.size() != 0)
		{
			send(client, input.data(), input.size(), 0);
		}
		if (select_recv(client)) {
			std::string recv_string;
			recv_string.reserve(512);
			recv(client, recv_string.data(), 512, 0);
			std::cout << recv_string;
		}
	}
#if defined(_WIN32) || defined(_WIN64)
	closesocket(client);
	WSACleanup();
#else
	close(client);
#endif
	return 0;
}
