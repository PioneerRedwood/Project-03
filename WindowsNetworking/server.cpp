// NewStart branch 
// Server programming
// 2020.07.15. ~ 

#include <iostream>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <string>
#include <sstream>

#define DEFAULT_BUFLEN 256
#define DEFAULT_PORT 65000
#define BACKLOG 10

#pragma comment(lib, "ws2_32.lib")

int main()
{
	using std::cout;
	using std::endl;

	WSADATA wsaData;
	int iResult;

	char buffer[DEFAULT_BUFLEN];
	
	// ���� ȯ�� �ʱ�ȭ
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		cout << ">> Error at WSAStartup()!\nexit()";
		exit(EXIT_FAILURE);
	}

	sockaddr_in hint;

	// Ŭ���̾�Ʈ ���� ���� ���� ����
	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ListenSocket == INVALID_SOCKET)
	{
		cout << ">> Error at making a ListenSocket!\nexit()";
		exit(EXIT_FAILURE);
	}

	// ��� �������� ����
	ZeroMemory(&hint, sizeof(hint));
	hint.sin_family = AF_INET;
	// htons: Host TO Network Short
	hint.sin_port = htons(DEFAULT_PORT);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	// TCP ��� ���� ����
	iResult = bind(ListenSocket, (sockaddr*)&hint, sizeof(hint));
	if (iResult == SOCKET_ERROR)
	{
		cout << ">> Error at bind()!\nexit()";
		exit(EXIT_FAILURE);
	}

	//// ���� �ּҿ� ��Ʈ ��ȣ
	//iResult = getaddrinfo(NULL, DEFAULT_PORT, &hint, &result);
	//if (iResult != 0)
	//{
	//	cout << ">> Error at getaddrinfo()!\nexit()";
	//	exit(EXIT_FAILURE);
	//}	


	// listen ����
	if (listen(ListenSocket, BACKLOG) == SOCKET_ERROR)
	{
		cout << ">> Error at listen()!\nexit()";
		exit(EXIT_FAILURE);
	}

	cout << ">> ������ ��ٸ��� �ֽ��ϴ�." << endl;

	fd_set master;
	FD_ZERO(&master);

	FD_SET(ListenSocket, &master);

	while (true)
	{
		fd_set copy = master;

		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];
			if (sock == ListenSocket)
			{
				// ���� ����� ������ ���
				SOCKET client = accept(ListenSocket, nullptr, nullptr);

				// ����Ǿ� �ִ� Ŭ���̾�Ʈ ����Ʈ�� �߰�
				FD_SET(client, &master);

				// ȯ�� �޽��� ����
				std::string welcomeMsg = ">> Welcome! Connected with Server\r\n";
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);

				// �ؾ��� ��: ����� �������� ���ӵ� Ŭ���̾�Ʈ ���� �����ֱ�
				
			}
			else
			{
				// ���ο� ������ ���
				ZeroMemory(buffer, DEFAULT_BUFLEN);
				iResult = recv(sock, buffer, DEFAULT_BUFLEN, 0);

				// ���ۿ� �� �� Ȯ�� �ʿ�
				std::cout << "SOCKET #" << sock << ": " << buffer;
				if (iResult <= 0)
				{
					// Ŭ���̾�Ʈ ���� ����
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					// �ٸ� Ŭ���̾�Ʈ�� ����, ����� �� ����
					// ������ �̻��� ���� �ʿ�
					for (unsigned i = 0; i < master.fd_count; i++)
					{
						SOCKET outSock = master.fd_array[i];
						if (outSock != ListenSocket && outSock != sock)
						{
							std::ostringstream ss;
							ss << "SOCKET #" << sock << ": " << buffer << "\r";
							std::string strOut = ss.str();

							send(outSock, strOut.c_str(), strOut.size() + 1, 0);
						}
					}
				}
			}
		}
	}

	// ����
	WSACleanup();
	return 0;
}