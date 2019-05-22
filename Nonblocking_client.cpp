// Windows socket 프로그래밍 클라이언트 코드 <파일 전송>
#define WIN32_LEAN_AND_MEAN

#include<iostream>
#include<string>
#include<ws2tcpip.h>
#include<winsock2.h>
#include<filesystem>
#include<fcntl.h>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 2048
#define DEFAULT_PORT "9000"

int main()
{
	WSADATA wsaData;
	int iResult;
	char buf[DEFAULT_BUFLEN];
	int bufLen = DEFAULT_BUFLEN;

	struct addrinfo* result = NULL, * ptr = NULL, hints;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult == INVALID_SOCKET)
	{
		cout << "WSAStartup failed: " << WSAGetLastError() << endl;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// 서버 주소와 포트를 설정
	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		cout << "getaddrinfo failed: " << iResult << endl;
		WSACleanup();
		return 1;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;
	ptr = result;

	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET)
	{
		cout << "Error at socket(): " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	if (fcntl(ConnectSocket, F_SETFL, O_NONBLOCK) == -1)
	{
		cout << "error at file Control: " << WSAGetLastError() << endl;
		return 1;
	}

	// 서버에 연결
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Unable to connect to server!" << endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	cout << "We'll start to sending a file.\n";

	FILE* fp;
	errno_t err;
	char fileName[20] = "Adata";
	err = fopen_s(&fp, fileName, "rb");		// 파일 열기 오류 검증 변수

	if (err == 0)
	{
		cout << "File open success!: " << fileName << endl;
		// 파일 크기 계산
		int iTest = fseek(fp, 0, SEEK_END);
		if (iTest != 0)
		{
			cout << "Error! failed to calculate: " << fileName << "'s Size..\n";
			return 1;
		}
		double fileSize = ftell(fp);
		cout << fileName << " size: " << fileSize << endl;
		fseek(fp, 0, SEEK_SET);
		double present = 0;
		double total = fileSize;

		do
		{
			if (iTest = recv(ConnectSocket, buf, bufLen, 0) == SOCKET_ERROR)
			{
				cout << "failed recv " << WSAGetLastError() << endl;
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			ZeroMemory(buf, bufLen);

			if ((total - present) < bufLen)
			{
				fread(buf, sizeof(char), total - present, fp);
				if ((iResult = send(ConnectSocket, buf, total - present, 0)) > 0)
				{
					cout << "file sending.. \n";
					if (iResult < (total - present))
					{
						fileSize -= iResult;
						present = ftell(fp);
						cout << "Sent: " << iResult << " byte (s)  present: " << (present / total) * 100 << "% " << endl;

						ZeroMemory(buf, bufLen);
						fread(buf, sizeof(char), (total - present), fp);
						if (iResult = send(ConnectSocket, buf, (total - present), 0) > 0)
						{
							fileSize -= iResult;
							present = ftell(fp);
							cout << "Sent: " << iResult << " byte (s)  present: " << (present / total) * 100 << "% " << endl;
						}
					}
					else
					{
						fileSize -= iResult;
						present = ftell(fp);
						cout << "Sent: " << iResult << " byte (s)  present: " << (present / total) * 100 << "% " << endl;
					}
				}
				else
				{
					cout << "Sending failed with error: " << WSAGetLastError() << endl;
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
			}
			else
			{
				fread(buf, sizeof(char), bufLen, fp);
				if ((iResult = send(ConnectSocket, buf, bufLen, 0)) > 0)
				{
					cout << "file sending.. \n";
					if (iResult < bufLen)
					{
						fileSize -= iResult;
						present = ftell(fp);
						cout << "Sent: " << iResult << " byte (s)  present: " << (present / total) * 100 << "% " << endl;

						ZeroMemory(buf, bufLen);
						fread(buf, sizeof(char), (total - present), fp);
						if (iResult = send(ConnectSocket, buf, (total - present), 0) > 0)
						{
							fileSize -= iResult;
							present = ftell(fp);
							cout << "Sent: " << iResult << " byte (s)  present: " << (present / total) * 100 << "% " << endl;
						}
					}
					else
					{
						fileSize -= iResult;
						present = ftell(fp);
						cout << "Sent: " << iResult << " byte (s)  present: " << (present / total) * 100 << "% " << endl;
					}
				}
				else
				{
					cout << "Sending failed with error: " << WSAGetLastError() << endl;
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
			}


		} while (present != total);
	}
	else
	{
		cout << "Opening " << fileName << " failed with error!\n";
		return 1;
	}


	// 더이상 전송할 데이터가 없을 때 소켓 닫기
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		cout << "shutdown failed: " << WSAGetLastError() << endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	// 종료
	closesocket(ConnectSocket);
	WSACleanup();
	return 0;
}