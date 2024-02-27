#ifndef CLIENT_H
#define CLIENT_H
#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <mutex>


#pragma comment (lib,  "Ws2_32.lib")

class Client
{
private:
	//std::string SERVER_IP = "192.168.1.77"; // Isaacs Laptop Home Wifi
	std::string SERVER_IP = "10.50.170.173"; // Isaacs Laptop TRU Wifi
	int LISTENING_SERVER_PORT = 50000;
	static const int BUFFER_SIZE = 1024;

	WSADATA wsaData;
	sockaddr_in server;
	SOCKET clientSock;

	std::mutex shutdownMutex;
	std::atomic <bool> shouldQuit = false;
	std::condition_variable shutdownCondition;


	// Initialzes Client
	void initializeClient()
	{
		// Initialize WSA 
		int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (res != 0)
			throw std::exception("[-] WSA startup failed");

		// Initialize socket
		clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (clientSock == INVALID_SOCKET)
			throw std::runtime_error("[-] Socket Creation Failed!");

		int flag = 1;
		if (setsockopt(clientSock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)) == SOCKET_ERROR)
			throw std::runtime_error("[-] setsockopt(TCP_NODELAY) Failed");

	}

	void shutdown()
	{
		closesocket(clientSock);
		WSACleanup();
	}

public:

	std::string RECV_MESSAGE_ERROR = "RECV_ERROR";

	Client()
	{
		try { initializeClient(); }
		catch (std::exception& e)
		{
			std::cerr << e.what();
		}
	}

	~Client()
	{
		shutdown();
	}

	void setServer_IP(std::string& server_ip)
	{
		// Initialize Server sockaddr_in struct
		server.sin_family = AF_INET; // Set IPv4
		server.sin_port = htons(LISTENING_SERVER_PORT); // Set server port to connect to
		inet_pton(AF_INET, server_ip.c_str(), &server.sin_addr); // Convert SERVER_IP into network byte order for transmission
	}

	void connectToServer()
	{
		// Connect to server
		int res = connect(clientSock, (sockaddr*)&server, sizeof(server));
		if (res == SOCKET_ERROR)
			throw std::runtime_error("[-] Connection Failed");
	}

	void sendMessage(std::string msg)
	{
		int res = send(clientSock, msg.c_str(), msg.length(), 0);
		if (res == SOCKET_ERROR)
			throw std::runtime_error("[-] Error: Message not sent!");
	}

	void sendInt(int i)
	{
		int networkInt = htonl(i);
		int res = send(clientSock, reinterpret_cast<char*>(&networkInt), sizeof(networkInt), 0);
		if (res == SOCKET_ERROR)
			throw std::runtime_error("[-] Error: Int not sent!");
	}

	// Receives messages
	std::string recvMessage()
	{
		char buffer[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE); // Zero buffer before receiving response

		int res = recv(clientSock, buffer, sizeof(buffer), 0);
		if (WSAGetLastError() == WSAETIMEDOUT || res == SOCKET_ERROR)
			return RECV_MESSAGE_ERROR; // Return error code

		return std::string(buffer, 0, res); // Return received message
	}
};
#endif

