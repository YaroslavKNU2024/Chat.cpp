#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <set>
#include <map>
#include <sstream>
#include <fstream>
#include <iomanip>

#pragma warning(disable: 4996)
using namespace std;

SOCKET Connection;

ofstream outputFileReceived;
ofstream outputFileSent;

void ClientHandler() {
	char msg[256];
	auto time = std::time(nullptr);

	while (true) {
		// Отримуємо повідомлення та записуємо їх в масив
		recv(Connection, msg, sizeof(msg), NULL);
		std::cout << msg << "\n\n";

		// ведеться запис отриманих повідомлень
		time = std::time(nullptr);
		outputFileReceived.open("logfile_received.txt", ofstream::out | ofstream::app);
		outputFileReceived << std::put_time(std::gmtime(&time), "%F %T") << "\t" << "Received: " << msg << "\n";
		outputFileReceived.close();
	}
}

int main(int argc, char* argv[]) {

	auto time = std::time(nullptr);
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	// IP адреса та номер порту
	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1133);
	addr.sin_family = AF_INET;

	// Створюємо з'єднання сокет
	Connection = socket(AF_INET, SOCK_STREAM, NULL);
	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
		std::cout << "Error: failed connect to server.\n";
		return 1;
	}
	std::cout << "Connected!\n";

	// Створюємо потік через бібліотеку winsock2.h
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

	char msg[256];
	while (true) {
		cin.getline(msg, sizeof(msg));
		if (msg[0] == '/') {
			send(Connection, "/", 2, NULL);
			outputFileReceived.close();
			outputFileSent.close();
			exit(0);
		}
		// Надсилаємо повідомленя Connection
		send(Connection, msg, sizeof(msg), NULL);
		// ведеться запис відправлених повідомлень
		outputFileSent.open("logfile_sent.txt", std::ofstream::out | std::ofstream::app);
		outputFileSent << std::put_time(std::gmtime(&time), "%F %T") << "\t" << "Sent: " << msg << "\n";
		outputFileSent.close();
		Sleep(10);
	}

	system("pause");
	return 0;
}
