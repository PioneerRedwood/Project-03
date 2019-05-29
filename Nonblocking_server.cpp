// Windows socket 프로그래밍 서버 코드 <다중 연결>
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

	// 통신 프로토콜 정보 설정
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// 서버 주소와 포트 번호를 등록
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
		shoutError("getaddrinfo failed!\n");

	// client 접속을 받을 소켓 생성
	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
		shoutError("socket error!\n");

	// TCP 통신 소켓 설정
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
		shoutError(result, ListenSocket, "bind() failed!\n");

	// listen 실행
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
			break;					// select() 오류시 루프 탈출
		else if (res == 0)
			continue;				// 시간 초과시 다음 루프 시작
		else
		{
			// 순차적으로 read의 fd_set의 변화를 탐색
			for (unsigned i = 0; i < read.fd_count; i++)
			{
				// 변화된 부분이 있다면 FD_ISSET은 양수를 반환
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

						// client 접속 내용 전시
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
							// 없다면 닫기
							FD_CLR(read.fd_array[i], &read);
							cout << "With " << read.fd_array[i] << " has been disconnected.\n";
							closesocket(temp.fd_array[i]);
						}
						else
						{
							// 받은 데이터를 다른 연결된 클라이언트에도 보내주기
							cout << naming(read.fd_array[i]) << ": " << buf << "__" << iResult << "byte (s)\n";
							
						}
					}
				}
			}
		}
	}
}