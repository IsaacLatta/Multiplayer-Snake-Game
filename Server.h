#ifndef SERVER_H
#define SERVER_H

#define WIN32_LEAN_AND_MEAN
#include <Winsock2.h>
#include <Ws2tcpip.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <mutex>
#include <functional>


#pragma comment (lib,  "Ws2_32.lib")

struct User
{
	std::atomic <bool> broadcastReady;
	SOCKET sock;

	User(SOCKET sock)
	{
		this->sock = sock;
		broadcastReady = false;
	}

	User(const User& user)
	{
		sock = user.sock;
		broadcastReady = user.broadcastReady.load();
	}
};


class Server
{
private:
	sockaddr_in server;
	WSADATA wsaData;
	SOCKET listeningSocket;
	const int LISTENING_SERVER_PORT = 50000; // Set to typically unused port
	static const int BUFFER_SIZE = 1024;

	std::mutex clientsMutex; 
	std::mutex shutdownMutex;

	std::vector <SOCKET> clients;
	std::vector <User> users;
	std::vector <std::thread> clientThreads;
	std::function <void(SOCKET)> clientHandler;

	std::condition_variable shutdownCondition;
	std::atomic <bool> stopReceivingConnections = false;
	std::atomic <bool> shouldQuit = false;
	std::atomic <bool> handlerSet = false;
	

	void initializeServer()
	{
		// Initialize WSA 
		int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (res == SOCKET_ERROR)
			throw std::exception("[-] WSA startup failed ");

		// Initialize Server sockaddr_in struct
		server.sin_family = AF_INET; // Set IPv4
		server.sin_port = htons(LISTENING_SERVER_PORT); // Set port to listen on
		server.sin_addr.s_addr = htonl(INADDR_ANY); // Listen on any address

		// Create and bind Socket
		listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		// Set sock option to allow connection in TIME_WAIT phase
		int yes = 1;
		if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes)) == SOCKET_ERROR)
			throw std::runtime_error(" [-] setsockopt(SO_REUSEADDR) Failed ");

		// Set sock opt to disable Nagles algorithm
		int flag = 1;
		if(setsockopt(listeningSocket, IPPROTO_TCP, TCP_NODELAY,(char*)&flag, sizeof(flag)) == SOCKET_ERROR)
			throw std::runtime_error("[-] setsockopt(TCP_NODELAY) Failed ");

		// Bind the listening socket
		res = bind(listeningSocket, (sockaddr*)&server, sizeof(server));
		if (res == SOCKET_ERROR)
			throw std::runtime_error("[-] Socket Bind Failed ");

		// Set Socket to listen
		res = listen(listeningSocket, SOMAXCONN);
		if (res == SOCKET_ERROR)
			throw std::runtime_error("[-] Socket Listen Failed ");
	}

	void shutdown()
	{
		shouldQuit = true;

		for (auto& thread : clientThreads)
		{
			thread.join();
		}

		for (auto& client : clients)
		{
			closesocket(client);
		}

		WSACleanup();
		closesocket(listeningSocket);
	}

public:
	std::string RECV_MESSAGE_ERROR = "RECV_ERROR";

	Server()
	{
		try { initializeServer(); }
		catch (std::exception& e)
		{
			std::cerr << e.what();
		}
	}

	~Server()
	{
		signalShutdown();
		shutdown();
	}

	bool pendingConnection()
	{
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(listeningSocket, &read_fds);

		// Set timeout to 3s
		struct timeval timeout;
		timeout.tv_sec = 3;  
		timeout.tv_usec = 0;

		// Check to see if any sockets are ready for reading
		// If listeningSocket is ready, it means a new connection is pending
		if (select(1, &read_fds, NULL, NULL, &timeout) > 0) 
		{
			// Check if listeningSocket is in the set
			if (FD_ISSET(listeningSocket, &read_fds)) 
			{
				return true; // New connection is pending
			}
		}

		// No pending connections or select error
		return false;
	}

	// Gets users connections
	SOCKET getConnection()
	{
		SOCKET clientSock;
		clientSock = accept(listeningSocket, NULL, NULL);
		if (clientSock == SOCKET_ERROR)
			return INVALID_SOCKET;

		users.push_back(User(clientSock));
		clients.push_back(clientSock);
		return clientSock;
	}

	std::string recvMessage(SOCKET sock)
	{
		char buffer[BUFFER_SIZE];
		int res = recv(sock, buffer, sizeof(buffer), 0);

		if (res == SOCKET_ERROR)
			return RECV_MESSAGE_ERROR;
		return std::string(buffer, 0, res);
	}

	int recvInt(SOCKET sock)
	{
		char buffer[sizeof(int)];
		int myRecvInt;
		int res = recv(sock, buffer, sizeof(buffer), 0);

		if (res == SOCKET_ERROR || res < sizeof(int))
			return -1;
			
		memcpy(&myRecvInt, buffer, sizeof(myRecvInt));
		return ntohl(myRecvInt);
	}

	void sendMessage(SOCKET sock, std::string& message )
	{
		int res = send(sock, message.c_str(), message.length(), 0);
		//if (res == SOCKET_ERROR)
			//throw std::runtime_error("[-] Error: Send Failed!");
	}
	
	void signalShutdown()
	{
		shouldQuit = true;
		stopReceivingConnections = true;
	}

	void setClientHandler(std::function <void(SOCKET)>& func)
	{
		handlerSet = true;
		clientHandler = std::move(func);
	}

	void allowBroadcast(SOCKET sock)
	{
		for (auto& user : users)
		{
			if (user.sock = sock)
				user.broadcastReady = true;
		}
	}

	// Relays message to all users
	void broadcastMessage(const std::string& message)
	{
		std::lock_guard <std::mutex> lock(clientsMutex);
	
		for (auto& user : users)
		{
			if (user.broadcastReady)
			{
				int res = send(user.sock, message.c_str(), message.length(), 0);
			}
		}
	}
};
#endif

