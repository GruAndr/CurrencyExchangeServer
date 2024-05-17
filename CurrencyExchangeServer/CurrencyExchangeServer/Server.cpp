#include "Server.h"


//Конструктор класса
Server::Server(std::string Ip, int Port)
{
	serverIp = Ip;
	serverPort = Port;

}


//--------------------------------------------//
//							                  //
//           Функции работы сервера           //
//							                  //
//--------------------------------------------//

//Функция запуска сервера
void Server::startServer()
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




	ServSock = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
	if (ServSock == INVALID_SOCKET) {
		std::cout << "Error initialization socket # " << WSAGetLastError() << std::endl;
		closesocket(ServSock);
#else
	if (ServSock == -1) {
		std::cout << "Error initialization socket # " << std::endl;
		close(ServSock);
#endif
#ifdef _WIN32
		WSACleanup();
#endif
	}
	else
		std::cout << "Server socket initialization is OK" << std::endl;



	sockaddr_in servInfo;


	in_addr ip_to_num;
	erStat = inet_pton(AF_INET, serverIp.c_str(), &ip_to_num);

	if (erStat != 1) {

		std::cout << "Error in IP translation to special numeric format" << std::endl;

	}

#ifdef _WIN32
	ZeroMemory(&servInfo, sizeof(servInfo));
#else
	memset(&servInfo, 0, sizeof(servInfo));
#endif


	servInfo.sin_family = AF_INET;
	servInfo.sin_addr = ip_to_num;
	servInfo.sin_port = htons(serverPort);

	erStat = bind(ServSock, reinterpret_cast<sockaddr*>(&servInfo), sizeof(servInfo));

#ifdef _WIN32
	if (erStat != 0) {
		std::cout << "Error Socket binding to server info. Error # " << WSAGetLastError() << std::endl;
		closesocket(ServSock);
#else
	if (erStat != 0) {
		std::cout << "Error Socket binding to server info. Error # " << std::endl;
		close(ServSock);
#endif
#ifdef _WIN32
		WSACleanup();
#endif
	}
	else
		std::cout << "Binding socket to Server info is OK" << std::endl;

	erStat = listen(ServSock, SOMAXCONN);


#ifdef _WIN32
	if (erStat != 0) {
		std::cout << "Can't start to listen to. Error # " << WSAGetLastError() << std::endl;
		closesocket(ServSock);
#else
	if (erStat != 0) {
		std::cout << "Can't start to listen to. Error # " << std::endl;
		close(ServSock);
#endif
#ifdef _WIN32
		WSACleanup();
#endif
	}
	else {
		std::cout << "Listening..." << std::endl;
	}


	std::thread th(&Server::handleConnections, this);

	while (true)
	{

		sockaddr_in clientInfo;

#ifdef _WIN32
		ZeroMemory(&clientInfo, sizeof(clientInfo));
#else
		memset(&clientInfo, 0, sizeof(clientInfo));
#endif


#ifdef _WIN32	
		int clientInfo_size = sizeof(clientInfo);
#else
		unsigned int clientInfo_size = sizeof(clientInfo);
#endif





		int ClientConn = accept(ServSock, (sockaddr*)&clientInfo, &clientInfo_size);


#ifdef _WIN32
		if (ClientConn == INVALID_SOCKET)
		{
			std::cout << "Client detected, but can't connect to a client. Error # " << WSAGetLastError() << std::endl;
			closesocket(ServSock);
			closesocket(ClientConn);
			WSACleanup();
		}
#else
		if (ClientConn == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				continue;
			}
			else
			{
				std::cout << "Client detected, but can't connect to a client. Error # " << std::endl;
				close(ServSock);
				close(ClientConn);
			}

		}
#endif

		else
		{
#ifdef _WIN32
			if (setNonBlocking(ClientConn) == SOCKET_ERROR) {
				closesocket(ServSock);
				closesocket(ClientConn);
				WSACleanup();
			}
#else
			if (setNonBlocking(ClientConn) == -1) {
				close(ServSock);
				close(ClientConn);
			}
#endif

			int code = sendMessage(ClientConn, "Welcome!Connection established!Please, log in or sign up.");
			connection clientConn;
			clientConn.clientSocket = ClientConn;



			connections.push_back(clientConn);

		}



	}




}

//Функция обработки подключений
void Server::handleConnections()
{

	while (true)
	{
		std::vector<char> message(100);
		for (int i = 0; i < connections.size(); i++)
		{
			int messageCode = recv(connections[i].clientSocket, message.data(), message.size(), 0);
			if (messageCode == -1)
			{
				continue;
			}
			else
			{
				if (messageCode != 0)
				{
					threads.emplace_back(&Server::handleMessage, this, i, message);

				}

			}


		}
	}


}

//Функция обработки принятого сообщения
void Server::handleMessage(int connId, std::vector<char> message)
{
	message.erase(std::remove(message.begin(), message.end(), '\0'), message.end());

	std::string serverAnsver;
	std::string clientMessage;
	clientMessage = std::string(message.begin(), message.end());
	std::cout << "Client message : " << clientMessage << std::endl;
	json o = json::parse(clientMessage)["response"];
	json answer;

	/*
	"response:"
	"action": "0 - login
			   1 - resister 
			   2 - sell   
			   3 - buy 
			   4 - change ballance 
			   5 - check sell requests 
			   6 - check buy requests"
	"arg1": "login/quantity"
	"arg2": "password/price"
	"authorised": "true/false"
	*/

	int action = std::stoi(std::string(o[0]["action"]));
	switch (action)
	{
	case 0:
		//login
		//arg1 == login
		//arg2 == password
	{
		std::string login = o[0]["arg1"];
		std::string password = o[0]["arg2"];
		int userId = userLogin(login, password);
		if (userId != -1)
		{
			connections[connId].isAuthorized = true;
			connections[connId].clientUserId = userId;
		}
		answer["code"] = "1";
		answer["answer"] = "Succesfully logged in! Your ID = " + std::to_string(connections[connId].clientUserId);
		int code = sendMessage(connections[connId].clientSocket, answer.dump());
		break;

	}
	case 1:
		//register
		//arg1 == login
		//arg2 == password
	{

		std::string login;
		std::string password;
		std::string userId = std::to_string(getLastID("users.json") + 1);
		int balance = 0;

		login = o[0]["arg1"];
		password = o[0]["arg2"];

		if (checkLogin(login) == false)
		{
			answer["code"] = "0";
			answer["answer"] = "Error.Login already in use!";

			int code = sendMessage(connections[connId].clientSocket, answer.dump());
			break;
		}
		json userData;
		userData["login"] = login;
		userData["password"] = password;

		json balanceData;
		balanceData["USD"] = "0";
		balanceData["RUB"] = "0";

		json newUser;
		newUser["ID"] = userId;
		newUser["data"] = userData;
		newUser["balance"] = balanceData;

		addUser(newUser,"users.json");
		answer["code"] = "1";
		answer["answer"] = "User registered";
		int code = sendMessage(connections[connId].clientSocket, answer.dump());
		break;
	}
	case 2:
		//sell
		//arg1 == quantity
		//arg2 == price
	{
		if (connections[connId].isAuthorized == false)
		{
			answer["code"] = "0";
			answer["answer"] = "You are not logged in!";
			int code = sendMessage(connections[connId].clientSocket, answer.dump());
			break;
		}
		std::string quantity = o[0]["arg1"];
		std::string price = o[0]["arg2"];
		if (std::stoi(quantity) <= 0 || std::stoi(price)<=0)
		{
			answer["code"] = "0";
			answer["answer"] = "Invalid price or quantity!";
			int code = sendMessage(connections[connId].clientSocket, answer.dump());
			break;
		}
		std::string bidId = std::to_string(getLastID("sell.json") + 1);

		json newBidInfo;
		newBidInfo["quantity"] = quantity;
		newBidInfo["price"] = price;

		json newBid;
		newBid["ID"] = bidId;
		newBid["data"] = newBidInfo;
		newBid["userId"] = std::to_string(connections[connId].clientUserId);
		addNewSellRequest(newBid,"sell.json");
		handleRequests("sell.json","buy.json","users.json");
		answer["code"] = "1";
		answer["answer"] = "Request added!";
		int code = sendMessage(connections[connId].clientSocket, answer.dump());
		break;
	}
	case 3:
		//buy
		//arg1 == quantity
		//arg2 == price
	{
		if (connections[connId].isAuthorized == false)
		{
			answer["code"] = "0";
			answer["answer"] = "You are not logged in!";
			int code = sendMessage(connections[connId].clientSocket, answer.dump());
			break;
		}



		std::string quantity = o[0]["arg1"];
		std::string price = o[0]["arg2"];

		if (std::stoi(quantity) <= 0 || std::stoi(price) <= 0)
		{
			answer["code"] = "0";
			answer["answer"] = "Invalid price or quantity!";
			int code = sendMessage(connections[connId].clientSocket, answer.dump());
			break;
		}

		std::string bidId = std::to_string(getLastID("buy.json") + 1);




		json newBidInfo;
		newBidInfo["quantity"] = quantity;
		newBidInfo["price"] = price;

		json newBid;
		newBid["ID"] = bidId;
		newBid["data"] = newBidInfo;
		newBid["userId"] = std::to_string(connections[connId].clientUserId);
		addNewBuyRequest(newBid,"buy.json");
		handleRequests("sell.json", "buy.json","users.json");
		answer["code"] = "1";
		answer["answer"] = "Request added!";
		int code = sendMessage(connections[connId].clientSocket, answer.dump());

		break;
	}
	case 4:
	{
		//change balance
		//arg1 == +-USD
		//arg2 == +-RUB
		if (connections[connId].isAuthorized == false)
		{
			answer["code"] = "0";
			answer["answer"] = "You are not logged in!";
			int code = sendMessage(connections[connId].clientSocket,answer.dump());
			break;
		}
		std::string usd = o[0]["arg1"];
		std::string rub = o[0]["arg2"];

		changeBalance(std::to_string(connections[connId].clientUserId), usd, rub,"users.json");

		answer["code"] = "1";
		answer["answer"] = "Your balance changed!";
		int code = sendMessage(connections[connId].clientSocket,answer.dump());

		break;
	}
	case 5:
	{
		if (connections[connId].isAuthorized == false)
		{
			answer["code"] = "0";
			answer["answer"] = "You are not logged in!";
			int code = sendMessage(connections[connId].clientSocket, answer.dump());
			break;
		}
		json currentBalance = json::parse(getBalance(std::to_string(connections[connId].clientUserId),"users.json"));

		answer["code"] = "1";
		answer["answer"] = "Your balance // USD : " + std::string(currentBalance["USD"]) +" // RUB : " + std::string(currentBalance["RUB"]);

		int code = sendMessage(connections[connId].clientSocket, answer.dump());


		break;
	}
	//Получить список заявок на продажу
	case 6:
	{
		if (connections[connId].isAuthorized == false)
		{
			answer["code"] = "0";
			answer["answer"] = "You are not logged in!";
			int code = sendMessage(connections[connId].clientSocket, answer.dump());
			break;
		}
		answer["code"] = "1";
		answer["answer"] = getRequests("sell.json");

		int code = sendMessage(connections[connId].clientSocket, answer.dump());

		break;

	}
	//Получить список заявок на покупку
	case 7:
	{
		if (connections[connId].isAuthorized == false)
		{
			answer["code"] = "0";
			answer["answer"] = "You are not logged in!";
			int code = sendMessage(connections[connId].clientSocket, answer.dump());
			break;
		}
		answer["code"] = "1";
		answer["answer"] = getRequests("buy.json");

		int code = sendMessage(connections[connId].clientSocket, answer.dump());

		break;
	}
	default:
	{
		answer["code"] = "0";
		answer["answer"] = "Error! Invalid message code!";
		int code = sendMessage(connections[connId].clientSocket, answer.dump());
		break;
	}
	}

}

//Функция установки флага неблокирующего сокета
int Server::setNonBlocking(int sockfd) {
#ifdef _WIN32
	unsigned long mode = 1;
	return ioctlsocket(sockfd, FIONBIO, &mode);
#else
	int flags = fcntl(sockfd, F_GETFL, 0);
	if (flags == -1) {
		return -1;
	}
	return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
#endif
}

//Функция отправки сообщения
int Server::sendMessage(int ClientConn, std::string message)
{
	short packet_size = 0;

	packet_size = send(ClientConn, message.data(), message.size(), 0);
	std::cout << "Server message : " << message << std::endl;
#ifdef _WIN32
	if (packet_size == SOCKET_ERROR) {
		return 1;
	}
#else
	if (packet_size == SO_ERROR) {
		return 1;
	}
#endif

	return 0;

}


//--------------------------------------------//
//											  //
//   Функции работы с файловой базой данных   //
//											  //
//--------------------------------------------//

//Функция обработки текущих заявок
int Server::handleRequests(std::string sellFileName,std::string buyFileName,std::string userFileName)
{
	std::ifstream inSell(sellFileName);
	std::ifstream inBuy(buyFileName);

	std::vector<std::vector<std::string>> deleteVector;
	std::vector<std::vector<std::string>> changeVector;


	std::string currentSellLine;
	std::string currentBuyLine;

	if (inSell.is_open())
	{
		if (inBuy.is_open())
		{

			if (std::getline(inSell, currentSellLine))
			{
				json sellRequest = json::parse(currentSellLine);

				int sellQuantity = std::stoi(std::string(sellRequest["data"]["quantity"]));
				int sellPrice = std::stoi(std::string(sellRequest["data"]["price"]));

				std::string sellUserId = sellRequest["userId"];

				if (std::getline(inBuy, currentBuyLine))
				{
					json buyRequest = json::parse(currentBuyLine);

					int buyQuantity = std::stoi(std::string(buyRequest["data"]["quantity"]));
					int buyPrice = std::stoi(std::string(buyRequest["data"]["price"]));
					std::string buyUserId = buyRequest["userId"];

					if (buyPrice >= sellPrice)
					{
						if (buyQuantity < sellQuantity)
						{
							sellQuantity = sellQuantity - buyQuantity;
							
							inSell.close();
							inBuy.close();
							deleteLine(buyRequest["ID"], buyFileName);
							changeQuantity(sellRequest["ID"], std::to_string(sellQuantity), sellFileName);
							changeBalance(std::string(buyUserId), std::to_string(buyQuantity), "-" + std::to_string(buyQuantity * buyPrice), userFileName);
							changeBalance(std::string(sellUserId), "-" + std::to_string(buyQuantity), std::to_string(buyQuantity * buyPrice), userFileName);
							return handleRequests(sellFileName,buyFileName, userFileName);

						}
						else
						{
							if (buyQuantity == sellQuantity)
							{
								inSell.close();
								inBuy.close();
								deleteLine(buyRequest["ID"], buyFileName);
								deleteLine(sellRequest["ID"], sellFileName);

								changeBalance(std::string(buyUserId), std::to_string(buyQuantity), "-" + std::to_string(buyQuantity * buyPrice), userFileName);
								changeBalance(std::string(sellUserId), "-" + std::to_string(buyQuantity), std::to_string(buyQuantity * buyPrice), userFileName);

								return handleRequests(sellFileName, buyFileName, userFileName);
								


							}
							else
							{
								if (buyQuantity > sellQuantity)
								{
									buyQuantity = buyQuantity - sellQuantity;

									inSell.close();
									inBuy.close();
									deleteLine(sellRequest["ID"], sellFileName);
									changeQuantity(buyRequest["ID"], std::to_string(buyQuantity), buyFileName);

									changeBalance(std::string(buyUserId), std::to_string(buyQuantity), "-" + std::to_string(buyQuantity * buyPrice), userFileName);
									changeBalance(std::string(sellUserId), "-" + std::to_string(buyQuantity), std::to_string(buyQuantity * buyPrice), userFileName);

									return handleRequests(sellFileName, buyFileName, userFileName);

								}
							}
						}
					}
					else
					{
						inSell.close();
						inBuy.close();
						return 0;
					}




				}

			}



		}
		inSell.close();
		inBuy.close();


	}
}

//Функция изменения баланса пользователя
int Server::changeBalance(std::string id,std::string usd, std::string rub,std::string fileName)
{
	std::ifstream in(fileName);

	std::ofstream outFile("bdtemp.json");
	if (in.is_open())
	{
		if (outFile.is_open())
		{
			std::string currentline;
			json currentUser;
			while (std::getline(in, currentline)) {
				currentUser = json::parse(currentline);
				if (currentUser["ID"] == id)
				{
					currentUser["balance"]["USD"] = std::to_string(std::stoi(std::string(currentUser["balance"]["USD"])) + std::stoi(usd));
					currentUser["balance"]["RUB"] = std::to_string(std::stoi(std::string(currentUser["balance"]["RUB"])) + std::stoi(rub));
					outFile << currentUser << std::endl;
				}
				else
				{
					outFile << currentline << std::endl;

				}


			}
		}

	}
	in.close();
	outFile.close();

	std::remove(fileName.c_str());
	if (std::rename("bdtemp.json", fileName.c_str()) == 0) {
		//std::cout << "Line changed." << std::endl;
		return 0;
	}
	else {
		//std::cerr << "Error!" << std::endl;
		return -1;
	}

	return 0;


}

//Функция удаления строки из файла
int Server::deleteLine(std::string id, std::string fileName)
{
	std::ifstream in(fileName);

	std::ofstream outFile("temp.json");
	if (in.is_open())
	{
		std::string currentline;
		json currentBid;
		while (std::getline(in, currentline)) {
			currentBid = json::parse(currentline);
			std::string currentId = currentBid["ID"];
			if (currentId != id)
			{
				outFile << currentline << std::endl;
			}
		}
	}

	in.close();
	outFile.close();


	std::remove(fileName.c_str());

	if (std::rename("temp.json", fileName.c_str()) == 0) {
		return 0;
	}
	else {
		return -1;
	}

	return 0;

}

//Функция получения последнего ID файла
int Server::getLastID(std::string fileName)
{

	std::ifstream in(fileName);//, std::ios::binary);
	std::string line;
	int lastRowId = 0;
	json lastLine;


	if (in.is_open())
	{
		while (std::getline(in, line))
		{
			if (line.size() > 1)
			{
				int entryId = std::stoi(std::string(json::parse(line)["ID"]));

				if (entryId > lastRowId)
				{
					lastRowId = entryId;
				}
			}
			else
			{
				in.close();
				return 0;
			}


		}
		in.close();
		return lastRowId;

	}
	else
	{
		return 0;
	}

}

//Функция добавления новой заявки на продажу валюты в файл
int Server::addNewSellRequest(json line,std::string fileName)
{
	std::ifstream in(fileName);//, std::ios::binary);

	int newPrice = std::stoi(std::string(line["data"]["price"]));

	std::ofstream outFile("selltemp.json");// , std::ios::binary);
	bool isAdded = false;
	if (in.is_open())
	{
		std::string currentline;
		json currentBid;
		while (std::getline(in, currentline)) {
			if(currentline.size()<=1)
			{
				outFile << line << std::endl;
				isAdded = true;
				outFile.close();
				in.close();
				break;
			}
			currentBid = json::parse(currentline);
			std::string currentId = currentBid["ID"];
			int currentPrice = std::stoi(std::string(currentBid["data"]["price"]));
			if (currentPrice > newPrice && isAdded == false)
			{
				outFile << line << std::endl;
				outFile << currentline << std::endl;
				isAdded = true;
			}
			else
			{
				outFile << currentline << std::endl;
			}
		}
		if (isAdded == false)
		{
			outFile << line << std::endl;
			isAdded = true;
			outFile.close();
		}
	}
	else
	{
		std::ofstream out;
		out.open(fileName, std::ios::app);

		if (out.is_open())
		{
			out << line << std::endl;
		}
		out.close();
	}
	in.close();
	outFile.close();

	if (isAdded == true)
	{
		std::remove(fileName.c_str());
	}
	if (isAdded && std::rename("selltemp.json", fileName.c_str()) == 0) {
		return 0;
	}
	else {
		return -1;
	}

	return 0;
}

//Функция добавления новой заявки на покупку валюты в файл
int Server::addNewBuyRequest(json line,std::string fileName)
{
	std::ifstream in(fileName);//, std::ios::binary);

	int newPrice = std::stoi(std::string(line["data"]["price"]));

	std::ofstream outFile("buytemp.json");// , std::ios::binary);
	bool isAdded = false;
	if (in.is_open())
	{
		std::string currentline;
		json currentBid;
		while (std::getline(in, currentline)) {
			if (currentline.size() <= 1)
			{
				outFile << line << std::endl;
				isAdded = true;
				outFile.close();
				in.close();
				break;
			}
			currentBid = json::parse(currentline);
			std::string currentId = currentBid["ID"];
			int currentPrice = std::stoi(std::string(currentBid["data"]["price"]));
			if (currentPrice < newPrice && isAdded == false)
			{
				outFile << line << std::endl;
				outFile << currentline << std::endl;
				isAdded = true;
			}
			else
			{
				outFile << currentline << std::endl;
			}
		}
		if (isAdded == false)
		{
			outFile << line << std::endl;
			isAdded = true;
			outFile.close();
		}
	}
	else
	{
		std::ofstream out;
		out.open(fileName, std::ios::app);

		if (out.is_open())
		{
			out << line << std::endl;
		}
		out.close();
	}
	in.close();
	outFile.close();

	if (isAdded == true)
	{
		std::remove(fileName.c_str());
	}
	if (isAdded && std::rename("buytemp.json", fileName.c_str()) == 0) {
		return 0;
	}
	else {
		return -1;
	}

	return 0;

}

//Функция добавления нового пользователя в файл
int Server::addUser(json line,std::string fileName)
{
	std::ofstream out;
	out.open(fileName, std::ios::app);

	if (out.is_open())
	{
		out << line<< std::endl;
	}
	out.close();

	return 0;
}

//Функция авторизации текущего пользователя. Возвращает ID найденного пользовалетя или -1 в случае неудачи.
int Server::userLogin(std::string login, std::string password)
{
	std::ifstream in("users.json");//, std::ios::binary);
	std::string line;
	json currentLine;
	if (in.is_open())
	{
		while (std::getline(in, line))
		{
			currentLine = json::parse(line);
			std::string userId = currentLine["ID"];
			if (currentLine["data"]["login"] == login)
			{
				if (currentLine["data"]["password"] == password)
				{
					return std::stoi(userId);
				}

			}

		}
		in.close();
		return -1;
	}


}

//Функция изменения объема заявки в файле
int Server::changeQuantity(std::string id, std::string quantity, std::string fileName)
{
	std::ifstream in(fileName);//, std::ios::binary);

	//int newPrice = std::stoi(std::string(line["data"]["price"]));

	std::ofstream outFile("temp.json");// , std::ios::binary);
	if (in.is_open())
	{
		std::string currentline;
		json currentBid;
		while (std::getline(in, currentline)) {
			currentBid = json::parse(currentline);
			std::string currentId = currentBid["ID"];
			if (currentId == id)
			{
				currentBid["data"]["quantity"] = quantity;
				outFile << currentBid << std::endl;
			}
			else
			{
				outFile << currentline << std::endl;
			}
		}
	}

	in.close();
	outFile.close();


	std::remove(fileName.c_str());
	if (std::rename("temp.json", fileName.c_str()) == 0) {

		return 0;
	}
	else {
		
		return -1;
	}

	return 0;

}

//Проверка на свободный логин
bool Server::checkLogin(std::string login)
{
	std::ifstream in("users.json");//, std::ios::binary);
	std::string line;
	json currentLine;
	if (in.is_open())
	{
		while (std::getline(in, line))
		{
			currentLine = json::parse(line);
			if (currentLine["data"]["login"] == login)
			{
				in.close();
				return false;
			}
		}
		in.close();
		return true;
	}


}

//Получение баланса пользователя
std::string Server::getBalance(std::string id,std::string fileName)
{
	std::ifstream in(fileName);//, std::ios::binary);
	std::string line;
	json currentLine;
	if (in.is_open())
	{
		while (std::getline(in, line))
		{
			currentLine = json::parse(line);
			if (currentLine["ID"] == id)
			{
				in.close();
				return currentLine["balance"].dump();
				
				
			}
		}
		in.close();
		return "Error";


	}


}

//Получение списка заявок
std::string Server::getRequests(std::string fileName)
{
	std::ifstream in(fileName);
	std::string line;
	std::string result;

	json currentLine;
	if (in.is_open())
	{
		while (std::getline(in, line))
		{
			currentLine = json::parse(line);
			result = result  + '\n' + "Request by user with id : " + std::string(currentLine["userId"]) + " /// Request ID : " + std::string(currentLine["ID"]) + "  ///   Quantity in USD : " + std::string(currentLine["data"]["quantity"]) + "  ///  Price in RUB : " + std::string(currentLine["data"]["price"]);
		
		}

	}
	else
	{

		return "None";
	}
	in.close();
	return result;

}

//Функция запуска тестов
void Server::startTest(int arg)
{
	switch (arg)
	{
	case 1:
	{
		std::cout << "Test 1" << std::endl;
		std::cout << "Sell requests = " << getRequests("sellTest.json") << std::endl;
		std::cout << "Buy requests = " << getRequests("buyTest.json") << std::endl;
		int quantity = 10;
		int price = 100;

		std::cout << "Adding new sell request quantity in USD = " << quantity << " // Price in RUB = " << price << std::endl;
		json request;
		request["ID"] = "1";
		json data;
		data["quantity"] = std::to_string(quantity);
		data["price"] = std::to_string(price);
		request["data"] = data;
		request["userId"] = "1";
		addNewSellRequest(request, "sellTest.json");

		quantity = 10;
		price = 80;
		std::cout << "Adding new buy request quantity in USD = " << quantity << " // Price in RUB = " << price << std::endl;
		
		request["ID"] = "1";
		
		data["quantity"] = std::to_string(quantity);
		data["price"] = std::to_string(price);
		request["data"] = data;
		request["userId"] = "1";
		addNewBuyRequest(request, "buyTest.json");

		std::cout << "Processing requests" << std::endl;
		handleRequests("sellTest.json", "buyTest.json", "bdTest.json");


		std::cout << "Sell requests = " << getRequests("sellTest.json") << std::endl;
		std::remove("sellTest.json");
		std::cout << "Buy requests = " << getRequests("buyTest.json") << std::endl;
		std::remove("buyTest.json");

	}
	case 2:
	{
		std::cout << "Test 2" << std::endl;
		std::cout << "Sell requests = " << getRequests("sellTest.json") << std::endl;
		std::cout << "Buy requests = " << getRequests("buyTest.json") << std::endl;
		int quantity = 10;
		int price = 100;

		std::cout << "Adding new sell request quantity in USD = " << quantity << " // Price in RUB = " << price << std::endl;
		json request;
		request["ID"] = "1";
		json data;
		data["quantity"] = std::to_string(quantity);
		data["price"] = std::to_string(price);
		request["data"] = data;
		request["userId"] = "1";
		addNewSellRequest(request, "sellTest.json");

		quantity = 20;
		price = 120;
		std::cout << "Adding new buy request quantity in USD = " << quantity << " // Price in RUB = " << price << std::endl;

		request["ID"] = "1";

		data["quantity"] = std::to_string(quantity);
		data["price"] = std::to_string(price);
		request["data"] = data;
		request["userId"] = "1";
		addNewBuyRequest(request, "buyTest.json");

		std::cout << "Processing requests" << std::endl;
		handleRequests("sellTest.json", "buyTest.json", "bdTest.json");


		std::cout << "Sell requests = " << getRequests("sellTest.json") << std::endl;
		std::remove("sellTest.json");
		std::cout << "Buy requests = " << getRequests("buyTest.json") << std::endl;
		std::remove("buyTest.json");


	}
	case 3:
	{
		std::cout << "Test 3" << std::endl;
		std::cout << "Sell requests = " << getRequests("sellTest.json") << std::endl;
		std::cout << "Buy requests = " << getRequests("buyTest.json") << std::endl;
		int quantity = 30;
		int price = 100;

		std::cout << "Adding new sell request quantity in USD = " << quantity << " // Price in RUB = " << price << std::endl;
		json request;
		request["ID"] = "1";
		json data;
		data["quantity"] = std::to_string(quantity);
		data["price"] = std::to_string(price);
		request["data"] = data;
		request["userId"] = "1";
		addNewSellRequest(request, "sellTest.json");

		quantity = 10;
		price = 120;
		std::cout << "Adding new buy request quantity in USD = " << quantity << " // Price in RUB = " << price << std::endl;

		request["ID"] = "1";

		data["quantity"] = std::to_string(quantity);
		data["price"] = std::to_string(price);
		request["data"] = data;
		request["userId"] = "1";
		addNewBuyRequest(request, "buyTest.json");
		request["ID"] = "2";
		std::cout << "Adding new buy request quantity in USD = " << quantity << " // Price in RUB = " << price << std::endl;
		addNewBuyRequest(request, "buyTest.json");
		request["ID"] = "3";
		std::cout << "Adding new buy request quantity in USD = " << quantity << " // Price in RUB = " << price << std::endl;
		addNewBuyRequest(request, "buyTest.json");
		request["ID"] = "4";
		std::cout << "Adding new buy request quantity in USD = " << quantity << " // Price in RUB = " << price << std::endl;
		addNewBuyRequest(request, "buyTest.json");

		std::cout << "Processing requests" << std::endl;
		handleRequests("sellTest.json", "buyTest.json", "bdTest.json");


		std::cout << "Sell requests = " << getRequests("sellTest.json") << std::endl;
		std::remove("sellTest.json");
		std::cout << "Buy requests = " << getRequests("buyTest.json") << std::endl;
		std::remove("buyTest.json");
	}
	default:
		break;
	}



}
