
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

std::string serverAnswerStr;
std::vector<std::string> message(2);
int ClientSock;

void getMessage()
{
	std::vector<char> serverAnswer(4096);

	send(ClientSock, message[0].data(), message[0].size(), 0);
	recv(ClientSock, serverAnswer.data(), serverAnswer.size(), 0);
	serverAnswer.erase(std::remove(serverAnswer.begin(), serverAnswer.end(), '\0'), serverAnswer.end());

	serverAnswerStr = std::string(serverAnswer.begin(), serverAnswer.end());
	std::cout << "Server answer : " << std::string(json::parse(serverAnswerStr)["answer"]) << std::endl;

}


std::string processMessage(json message)
{
	std::string result;
	result = message.dump();
	result.insert(result.begin() + 12, '[');
	result.insert(result.end() - 1, ']');

	return result;
}

void handleMessage()
{

	json response;
	json info;

	info["action"] = "-1";

	response["response"] = info;

	switch (std::stoi(message[1]))
	{
		//Стартовая страница меню
	case 0:
	{
		std::cout << std::endl << "                     ///   WELCOME PAGE  ///" << std::endl;
		std::cout << "Please authorize or register new user!\n	1 - authorize \n	2 - register new user \nPlease choose an option: ";
		std::cin >> message[0];
		message[1] = "1";
		std::cout << "                     /// --------------- ///" << std::endl;
		return handleMessage();
		break;
	}
	//Страница действия стартового меню
	case 1:
	{

		switch (std::stoi(message[0]))
		{
			//Авторизация
		case 1:
		{
			std::string login;
			std::string password;
			std::cout << std::endl << "                     ///    AUTHORIZATION ///" << std::endl;
			std::cout << "Please enter login : ";
			std::cin >> login;
			std::cout << "Please enter password : ";
			std::cin >> password;
			std::cout << "                     /// --------------- ///" << std::endl;
			info["action"] = "0";
			info["arg1"] = login;
			info["arg2"] = password;

			response["response"] = info;

			message[0] = processMessage(response);


			getMessage();
			if (json::parse(serverAnswerStr)["code"] == "1")
			{
				message[1] = "2";
			}
			else
			{
				message[1] = "0";
			}

			return handleMessage();
			break;

		}
		//Регистрация
		case 2:
		{

			std::string login;
			std::string password;
			std::cout << std::endl << "                     ///   REGISTRATION   ///" << std::endl;
			std::cout << "Please enter new login : ";
			std::cin >> login;
			std::cout << "Please enter new password : ";
			std::cin >> password;
			std::cout << "                     /// --------------- ///" << std::endl;

			info["action"] = "1";
			info["arg1"] = login;
			info["arg2"] = password;

			response["response"] = info;

			message[0] = processMessage(response);
			getMessage();
			message[1] = "0";


			return handleMessage();
			break;

		}
		default:
		{
			std::cout << "Error! Invalid argument!" << std::endl;
			message[1] = "0";
			return handleMessage();
			break;
		}
		}

	}
	//Главная страница меню
	case 2:
	{
		std::cout << std::endl << "                     ///    MAIN MENU    ///" << std::endl;
		std::cout << "Welcome!\n	1 - add new sell request\n	2 - add new buy request\n	3 - change balance\n	4 - check balance\n	5 - view sell requests\n	6 - view buy requests\nPlease choose an option: ";
		std::cin >> message[0];
		message[1] = "3";
		std::cout << "                     /// --------------- ///" << std::endl;

		return handleMessage();
		break;



	}
	//Страница действия главного меню
	case 3:
	{
		switch (std::stoi(message[0]))
		{
			//Добавление заявки на продажу валюты
		case 1:
		{
			std::string quantity;
			std::string price;
			std::cout << std::endl << "                     /// NEW SELL REQUEST ///" << std::endl;
			std::cout << "Please enter quantity in USD : ";
			std::cin >> quantity;
			std::cout << "Please enter price in RUB : ";
			std::cin >> price;
			std::cout << "                     /// --------------- ///" << std::endl;
			info["action"] = "2";
			info["arg1"] = quantity;
			info["arg2"] = price;

			response["response"] = info;

			message[0] = processMessage(response);
			getMessage();
			message[1] = "2";

			return handleMessage();
			break;

		}
		//Добавление заявки на покупку валюты
		case 2:
		{
			std::string quantity;
			std::string price;
			std::cout << std::endl << "                     /// NEW BUY REQUEST ///" << std::endl;
			std::cout << "Please enter quantity in USD : ";
			std::cin >> quantity;
			std::cout << "Please enter price in RUB : ";
			std::cin >> price;
			std::cout << "                     /// --------------- ///" << std::endl;
			info["action"] = "3";
			info["arg1"] = quantity;
			info["arg2"] = price;

			response["response"] = info;

			message[0] = processMessage(response);
			getMessage();
			message[1] = "2";

			return handleMessage();
			break;

		}
		//Изменение баланса
		case 3:
		{
			std::string USD;
			std::string RUB;
			std::cout << std::endl << "                     /// BALANCE CHANGE ///" << std::endl;
			std::cout << "Please enter USD change (+-) : ";
			std::cin >> USD;
			std::cout << "Please enter RUB change (+-) : ";
			std::cin >> RUB;
			std::cout << "                     /// --------------- ///" << std::endl;

			info["action"] = "4";
			info["arg1"] = USD;
			info["arg2"] = RUB;

			response["response"] = info;

			message[0] = processMessage(response);
			getMessage();
			message[1] = "2";

			return handleMessage();
			break;
		}
		//Получение баланса
		case 4:
		{
			std::cout << std::endl << "                     ///     BALANCE     ///" << std::endl;
			info["action"] = "5";
			info["arg1"] = "";
			info["arg2"] = "";

			response["response"] = info;
			message[0] = processMessage(response);
			getMessage();
			message[1] = "2";
			std::cout << "                     /// --------------- ///" << std::endl;
			return handleMessage();
			break;
		}
		case 5:
		{
			std::cout << std::endl << "                     ///  SELL REQUESTS  ///" << std::endl;
			info["action"] = "6";
			info["arg1"] = "";
			info["arg2"] = "";

			response["response"] = info;
			message[0] = processMessage(response);
			getMessage();
			message[1] = "2";
			std::cout << "                     /// --------------- ///" << std::endl;
			return handleMessage();
			break;
		}
		case 6:
		{
			std::cout << std::endl << "                     ///  BUY REQUESTS  ///" << std::endl;
			info["action"] = "7";
			info["arg1"] = "";
			info["arg2"] = "";

			response["response"] = info;
			message[0] = processMessage(response);
			getMessage();
			message[1] = "2";
			std::cout << "                     /// --------------- ///" << std::endl;
			return handleMessage();
			break;
		}
		default:
		{
			std::cout << "Error! Invalid argument!" << std::endl;
			message[1] = "2";
			return handleMessage();
			break;
		}
		}

	}
	default:
	{
		std::cout << "Error! Invalid argument!" << std::endl;
		message[1] = "0";
		return handleMessage();
		break;
	}
	}




}




int main()
{
	int erStat;
#ifdef _WIN32
	WSADATA wsData;
	erStat = WSAStartup(MAKEWORD(2, 2), &wsData);
	if (erStat != 0) {
		std::cout << "Error WinSock version initializaion #";
		std::cout << WSAGetLastError();
	}
	else
		std::cout << "WinSock initialization is OK" << std::endl;
#endif
	sockaddr_in servInfo;
	ClientSock = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
	ZeroMemory(&servInfo, sizeof(servInfo));
#else
	memset(&servInfo, 0, sizeof(servInfo));
#endif


	in_addr ip_to_num;
	std::string serverIp = "127.0.0.1";
	std::cout << "Please, enter server ip: ";
	std::cin >> serverIp;
	erStat = inet_pton(AF_INET, serverIp.c_str(), &ip_to_num);


	std::string serverPort = "1234";
	std::cout << "Please, enter server port: ";
	std::cin >> serverPort;

	servInfo.sin_family = AF_INET;
	servInfo.sin_addr = ip_to_num;
	servInfo.sin_port = htons(std::stoi(serverPort));

	erStat = connect(ClientSock, (sockaddr*)&servInfo, sizeof(servInfo));
#ifdef _WIN32
	if (erStat != 0) {
		std::cout << "Connection to Server is FAILED. Error # " << WSAGetLastError() << std::endl;
		closesocket(ClientSock);
		WSACleanup();
		return 1;
	}
#else
	if (erStat != 0) {
		std::cout << "Connection to Server is FAILED. Error # " << std::endl;
		close(ClientSock);
	}
#endif
	else
	{
		std::string clientMessage;
		std::vector<char> serverAnswer(100);

		recv(ClientSock, serverAnswer.data(), serverAnswer.size(), 0);
		serverAnswer.erase(std::remove(serverAnswer.begin(), serverAnswer.end(), '\0'), serverAnswer.end());

		serverAnswerStr = std::string(serverAnswer.begin(), serverAnswer.end());

	}
	message[1] = "0";
	handleMessage();



}






