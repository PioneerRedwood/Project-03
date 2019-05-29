// Windows socket ���α׷��� ���� �ڵ� <���� ����>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<iostream>

#define DEFAULT_BUFLEN 256
#define DEFAULT_PORT "9000"
#define BACKLOG 10

#pragma comment (lib, "ws2_32.lib")		

using namespace std;

void shoutError(string errormsg)
{
	cout << errormsg << endl;
	WSACleanup();
	exit(1);
}

void shoutError(addrinfo* res, SOCKET sock, string errormsg)
{
	cout << errormsg << endl;
	freeaddrinfo(res);
	closesocket(sock);
	WSACleanup();
	exit(1);
}

string naming(unsigned i)
{
	if (i % 2)
		return "Doublen";
	else if (i % 3)
		return "Trillon";
	else if (i % 5)
		return "Fiphon";
	else if (i % 7)
		return "Lockiln";
}

int main()
{
	WSADATA wsadata;
	int iResult;

	char buf[DEFAULT_BUFLEN];
	int bufLen = DEFAULT_BUFLEN;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (iResult != 0)
		shoutError("WSAStartup failed!\n");

	struct addrinfo* result = NULL, hints;

	// ��� �������� ���� ����
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// ���� �ּҿ� ��Ʈ ��ȣ�� ���
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
		shoutError("getaddrinfo failed!\n");

	// client ������ ���� ���� ����
	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
		shoutError("socket error!\n");

	// TCP ��� ���� ����
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
		shoutError(result, ListenSocket, "bind() failed!\n");

	// listen ����
	if (listen(ListenSocket, BACKLOG) == SOCKET_ERROR)
		shoutError(result, ListenSocket, "listen() failed!\n");

	cout << "Wating for Connection.. " << endl;

	struct timeval timeout;
	fd_set read, temp;
	int res;

	FD_ZERO(&read);
	FD_SET(ListenSocket, &read);

	while (true)
	{
		temp = read;

		timeout.tv_sec = 5;
		timeout.tv_usec = 50000;

		res = select(NULL, &temp, NULL, NULL, &timeout);
		if (res == SOCKET_ERROR)
			break;					// select() ������ ���� Ż��
		else if (res == 0)
			continue;				// �ð� �ʰ��� ���� ���� ����
		else
		{
			// ���������� read�� fd_set�� ��ȭ�� Ž��
			for (unsigned i = 0; i < read.fd_count; i++)
			{
				// ��ȭ�� �κ��� �ִٸ� FD_ISSET�� ����� ��ȯ
				if (FD_ISSET(read.fd_array[i], &temp))
				{
					if (ListenSocket == read.fd_array[i])
					{
						sockaddr_in client;
						int clientSize = sizeof(client);
						SOCKET clientSocket = accept(ListenSocket, (sockaddr*)& client, &clientSize);
						if (clientSocket == INVALID_SOCKET)
						{
							shoutError(result, clientSocket, "accept() failed!\n");
						}
						FD_SET(clientSocket, &read);

						// client ���� ���� ����
						char host[NI_MAXHOST];
						char service[NI_MAXSERV];

						ZeroMemory(host, NI_MAXHOST);
						ZeroMemory(service, NI_MAXSERV);

						if (getnameinfo((sockaddr*)& client, sizeof(client), host, NI_MAXHOST,
							service, NI_MAXSERV, 0) == 0)
						{
							cout << host << " connected on port " << service << endl;
						}
						else
						{
							inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
							cout << host << "connected on port " << service << endl;
						}
					}
					else
					{
						ZeroMemory(buf, bufLen);
						iResult = recv(read.fd_array[i], buf, bufLen, 0);
						if (iResult <= 0)
						{
							// ���ٸ� �ݱ�
							FD_CLR(read.fd_array[i], &read);
							cout << "With " << read.fd_array[i] << " has been disconnected.\n";
							closesocket(temp.fd_array[i]);
						}
						else
						{
							// ���� �����͸� �ٸ� ����� Ŭ���̾�Ʈ���� �����ֱ�
							cout << naming(read.fd_array[i]) << ": " << buf << "__" << iResult << "byte (s)\n";
							
						}
					}
				}
			}
		}
	}
}