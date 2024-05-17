#pragma once
#include <iostream>
#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <cstring>
#endif
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <thread>
#include "json.hpp"
using json = nlohmann::json;

class Server
{
public:
	Server(std::string Ip, int Port);

	void startServer();

	void handleConnections();

	void handleMessage(int connId, std::vector<char> message);

	int changeQuantity(std::string id,std::string quantity,std::string fileName);

	int deleteLine(std::string id,std::string fileName);

	int handleRequests(std::string sellFileName, std::string buyFileName,std::string userFileName);

	int setNonBlocking(int sockfd);

	int sendMessage(int ClientConn, std::string message);

	int addUser(json line,std::string fileName);

	int getLastID(std::string fileName);

	int changeBalance(std::string id,std::string usd,std::string rub,std::string fileName);

	int addNewSellRequest(json line, std::string fileName);

	int addNewBuyRequest(json line,std::string fileName);

	int userLogin(std::string login,std::string password);

	std::string getBalance(std::string id,std::string fileName);

	std::string getRequests(std::string fileName);

	bool checkLogin(std::string login);

	void startTest(int arg);

private:
	struct connection
	{
		int clientSocket;
		int clientUserId = 0;
		bool isAuthorized = false;
	};
	std::vector<std::thread> threads;
	std::vector<connection> connections;
	std::string serverIp;
	int serverPort;

	int ServSock;
};

