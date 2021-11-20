#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <fstream>
#include <iomanip>

using namespace std;
#pragma warning(disable: 4996)

// Оголошуємо сокет
SOCKET Connection;

ofstream outputFileReceived;
ofstream outputFileSent;

// Клас елементів, типів та їх значень
class MyClass {
public:
	MyClass(string name, string type, string value) {
		this->name = name;
		this->type = type;
		this->value = value;
	}
	string name, type, value;
};

map<string, MyClass*> m;

// Функція, яка розбиває рядок на команди (лексеми)
pair<int, vector<string>> parser(string TextNotParsed) {
	stringstream RawText(TextNotParsed);
	string segment;
	vector<string> seglist;

	while (getline(RawText, segment, ';')) {
		seglist.push_back(segment);
	}
	return make_pair(seglist.size(), seglist);
}

// програма, яка відправляє клієнту повідомлення та записує, у log-файл дану інформацію
void sendCommand(string x) {
	char c[256] = { '\0' };
	for (int i = 0, len = x.length(); i < len; ++i) c[i] = x[i];
	send(Connection, c, 256, NULL);
	auto time = std::time(nullptr);
	outputFileSent << std::put_time(std::gmtime(&time), "%F %T") << "\t";
	outputFileSent << "Sent: " << c << "\n";
	outputFileSent.close();
}

void ClientHandler(int index) {
	char msg[256] = { '\0' };
	auto time = std::time(nullptr);
	// Починаємо цикл обробки команд
	while (true) {
		// отримуємо повідомлення
		recv(Connection, msg, sizeof(msg), NULL);
		if (msg[0] == '/') {
			outputFileReceived.close();
			outputFileSent.close();
			exit(0);
		}
		time = std::time(nullptr);
		outputFileSent.open("logfile_sent.txt", std::ofstream::out | std::ofstream::app);
		// переведення тексту у string для легшої обробки
		string s_copy(msg);
		if (s_copy.length() > 256) {
			outputFileSent << std::put_time(std::gmtime(&time), "%F %T") << "\t" << "ERROR!!! CHARACTER LIMITEXCEEDED. MESSAGE NOT RECEIVED!!!\n";
			outputFileSent.close();
			sendCommand("Error while receiving a message!");
		}
		// Записуємо отримане повідомлення в лог файл
		time = std::time(nullptr);
		outputFileReceived.open("logfile_received.txt", ofstream::out | ofstream::app);
		outputFileReceived << std::put_time(std::gmtime(&time), "%F %T") << "\t" << "Received: " << msg << "\n";
		outputFileReceived.close();

		outputFileSent.open("logfile_sent.txt", std::ofstream::out | std::ofstream::app);

		pair<int, vector<string>> p = parser(s_copy);
		if (p.first == 0)
			sendCommand("You haven't entered anything. Try again\n");
		// перевіряються випадки введення різних команд
		else if (p.first == 1) {
			if (p.second[0] == "who")
				sendCommand("Hello there! My name is Yaroslav and this is my first server program that immitates the workof the cloud. You can check details in my project report.\n");
			else if (!(m.count(p.second[0]))) 
				sendCommand("ERROR!!! VARIABLE DOES NOT YET EXIST! CREATE ONE!");
			else
				sendCommand(m[p.second[0]]->name + ";" + m[p.second[0]]->type + ";" + m[p.second[0]]->value);
		}
		else if (p.first == 3) {
			try {
				MyClass * C = new MyClass(p.second[0], p.second[1], p.second[2]);
				m[p.second[0]] = C;
				sendCommand("New element added / rewritten to the list");
			}
			catch (exception) {
				sendCommand("FATAL ERROR!!!\n");
			}
		}
		else
			sendCommand("ERROR!!! No such command determined\n");
	}
}

int main(int argc, char* argv[]) {

	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1133);
	addr.sin_family = AF_INET;

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
	listen(sListen, SOMAXCONN);

	SOCKET newConnection;
	newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

	if (newConnection == 0) {
		cout << "Error #2\n";
		exit(1);
	}

	cout << "Client Connected!\n";
	Connection = newConnection;
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

	system("pause");
	return 0;
}