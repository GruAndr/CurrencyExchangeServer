#include "Server.cpp"



int main()
{
	std::string ip;
	std::string port;
	std::cout << "Hello!Please, enter ip: ";
	std::cin >> ip;
	std::cout << "Ok. Now, please, enter server port: ";
	std::cin >> port;

	Server Myserver(ip, std::stoi(port));

	std::string command;
	std::cout << "Please choose an option:\n	1 - start server\n	2 - start test1\n	3 - start test2\n	4 - start test3\nEnter: ";
	std::cin >> command;

	switch (std::stoi(command))
	{
	case 1:
	{
		Myserver.startServer();
	}
	case 2:
	{
		Myserver.startTest(1);
		break;
	}
	case 3:
	{
		Myserver.startTest(2);
		break;

	}
	case 4:
	{
		Myserver.startTest(3);
		break;

	}
	default:
	{
		std::cout << "Invalid argument!";
		break;
	}
	}



}


