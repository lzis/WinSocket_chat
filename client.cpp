#include<WinSock2.h>
#include<iostream>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
#define BUF_SIZE 2048
#define HANDLE_SIZE 8
DWORD WINAPI recvThread(LPVOID param);//������Ϣ�߳�
DWORD WINAPI sendThread(LPVOID param);//������Ϣ�߳�


typedef struct User
{
	int type;
	char id[9];
	char password[17];
	SOCKET user_socket;
}User;

typedef struct Message
{
	int type;
	char origin_userId[9];
	char des_userId[9];
	char messsage[1024];
}Message;

typedef struct Friend
{
	int type;
	User FriendList[1024];
}Friend;


User friendList[1024];
int friendNum = 0;
int isLogin = 0;

int main()
{
	int retVal;
	//��ʼ�������������ͻ����׽���
	WSADATA wsaDate;
	WSAStartup(MAKEWORD(2, 2), &wsaDate);
	SOCKET cSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN serverAddr;
	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(9990);


	retVal = connect(cSocket, (PSOCKADDR)&serverAddr, sizeof(serverAddr));
	if (retVal == SOCKET_ERROR)
	{
		cout << "������쳣������ʧ��" << endl;
		closesocket(cSocket);
		WSACleanup();
		return -1;
	}
	User user;
	memset(&user, 0, sizeof(User));
	user.user_socket = cSocket;
	HANDLE threadHandles[HANDLE_SIZE];
	threadHandles[0] = CreateThread(NULL, 0, recvThread, &user, 0, NULL);//���׽��ֵ�ַ��Ϊ���������ݸ��߳�

two:
	cout << "==========welcome to chat !!============" << endl;
	cout << "���������ѡ��" << endl;
	cout << "1 ��¼ 2 ע�� " << endl;
	int op; cin >> op;

	if (op == 1)
	{
	one:
		system("cls");
		cout << "============��¼=============" << endl;
		cout << "id:" << endl;
		char id[9]; char password[17];
		cin >> id;
		cout << "password:" << endl;
		cin >> password;
		strcpy(user.id, id);
		strcpy(user.password, password);
		user.type = 1;
		char ch[sizeof(User)];
		memcpy(ch, &user, sizeof(User));
		retVal = send(cSocket, ch, sizeof(ch), 0);

		Sleep(2000);
		if (isLogin == 0)
		{
			goto one;
		}
		else
		{
			char sendBuf[sizeof(Message)];
			Message friends;
			memset(&friends, 0, sizeof(Message));
			friends.type = 10;
			memcpy(sendBuf, &friends, sizeof(Message));
			send(cSocket, sendBuf, sizeof(Message), 0);
			//system("cls");
			cout << "===========��ӭ�û���" << user.id << "����¼==============" << endl;
		}
	}
	else if (op == 2)//ע��
	{
		cout << "===========ע��============" << endl;
		cout << "����������û�id(������8λ)��" << endl;
		char id1[9]; cin >> id1;
		cout << "�������������(������16λ����" << endl;
		char password1[17]; cin >> password1;
		User user1;
		memset(&user1, 0, sizeof(User));
		strcpy(user1.id, id1);
		strcpy(user1.password, password1);
		user1.type = 2;
		char ch1[sizeof(User)];
		memcpy(ch1, &user1, sizeof(User));
		retVal = send(cSocket, ch1, sizeof(ch1), 0);
		Sleep(2000);
		system("cls");
		goto two;
	}
	else
	{
		cout << "��������" << endl;
		goto two;
	}
	

	threadHandles[1] = CreateThread(NULL, 0, sendThread, &user, 0, NULL);//���׽��ֵ�ַ��Ϊ���������ݸ��߳�
	WaitForMultipleObjects(2, threadHandles, true, INFINITE);//�ȴ��߳̽���
	closesocket(cSocket);
	WSACleanup();
	return 0;
}
char recvBuf[BUF_SIZE];
int isOnline = 0;

DWORD WINAPI sendThread(LPVOID param)//������Ϣ�߳�
{
	User user = *(User*)param;
	char buf[sizeof(Message)];
	char chatUser[9];
	while (true)
	{
		ZeroMemory(buf, sizeof(Message));
		cout << "1ѡ��������� 2 ��Ӻ��� 3 ����Է�ID�������� 4 ������" << endl;
		int op; cin >> op;
		if (op == 1)
		{
			cout << "��ĺ����б�" << endl;
			for (int i = 0; i < friendNum; i++)
			{
				cout <<i<<":" << " ��" << friendList[i].id << "��" << " ";
			}
			cout << endl;
			cout << "��ѡ����ѣ�" << endl;
			int i; cin >> i;
			strcpy(chatUser, friendList[i].id);
			goto three;
		}
		else if (op == 2)
		{
			
			char sendBuf[sizeof(Message)];
			Message friends;
			memset(&friends, 0, sizeof(Message));
			friends.type = 20;
			cout << "��������Ӻ���ID��" << endl;
			cin >> friends.des_userId;
			memcpy(sendBuf, &friends, sizeof(Message));
			send(user.user_socket, sendBuf, sizeof(Message), 0);
			Sleep(2000);

		}
		else if (op == 3)
		{
			Message message;
			memset(&message, 0, sizeof(Message));
			cout << "�������" << endl;
			cin >> chatUser;
	three:	strcpy(message.des_userId, chatUser);
			message.type = 0;//��֤�Է��Ƿ�����
			memcpy(buf, &message, sizeof(Message));
			send(user.user_socket, buf, sizeof(buf), 0);
			Sleep(2000);
			if (isOnline == 1)
			{
				char chatUserId[9];
				strcpy(chatUserId, message.des_userId);
				system("cls");
				cout << "==================���ں��û���" << message.des_userId << "����������===================" << endl;
				while (1)
				{
					ZeroMemory(buf, sizeof(Message));
					strcpy(message.des_userId, chatUserId);
					cin >> message.messsage;
					if (strcmp(message.messsage, "quit") == 0)
						break;
					message.type = 1;
					strcpy(message.origin_userId, user.id);
					memcpy(buf, &message, sizeof(Message));

					int retVal = send(user.user_socket, buf, sizeof(buf), 0);
					
					if (retVal == SOCKET_ERROR)
					{
						if (WSAGetLastError() == WSAECONNRESET) break;
					}
				}
				isOnline = 0;
			}
		}
		else if (op == 4)
		{

			Message message;
			memset(&message, 0, sizeof(Message));
			ZeroMemory(buf, sizeof(Message));
			message.type = 99;
			strcpy(message.origin_userId, user.id);
			memcpy(buf, &message, sizeof(Message));
			send(user.user_socket, buf, sizeof(buf), 0);
			system("cls");
			cout << "============������(����quit�˳�)===========" << endl;
			while (1)
			{
				Message message;
				memset(&message, 0, sizeof(Message));
				ZeroMemory(buf, sizeof(Message));
				message.type = 100;
				cin >> message.messsage;
				if (strcmp(message.messsage, "quit") == 0)
					break;
				strcpy(message.origin_userId, user.id);
				memcpy(buf, &message, sizeof(Message));
				int retVal = send(user.user_socket, buf, sizeof(buf), 0);
			}
			

		}
		else
		{
			cout << "�������" << endl;
		}
		system("cls");
	}

	return 0;
}
DWORD WINAPI recvThread(LPVOID param)
{
	User user = *(User*)param;
	while (true)
	{
		ZeroMemory(recvBuf, BUF_SIZE);
		int retVal = recv(user.user_socket, recvBuf, BUF_SIZE, 0);
		if (retVal == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAECONNRESET)//�Է�ͨ���ѹر�
			{
				cout << "�������ѹرգ��ͻ���ͨѶ�ж�" << endl;
				break;
			}
			else if (retVal == 0)//�Է�ͨ���ѹر�
			{
				printf("The Server End closed!\n");
				break;
			}
		}
		Message* message = (Message*)recvBuf;
		if (message->type == -1 )
		{
			cout << message->messsage << endl;
			if (strcmp(message->messsage, "�Է�����") == 0)
			    isOnline = 1;
			if (strcmp(message->messsage, "��¼�ɹ�") == 0)
				isLogin = 1;
			if (strcmp(message->messsage, "��ӳɹ�") == 0)
			{
				strcpy(friendList[friendNum++].id, message->des_userId);
			}
		}
		else if (message->type == 10)
		{
			User* friends = (User*)message->messsage;
			for (int i = 2; i < friends[0].type; i++)
			{
				friends = (User*)(message->messsage+i*56);
				strcpy(friendList[friendNum++].id , friends->id);
			}

		}

		else
		{
			cout <<"��"<<message->origin_userId<<"����"<< message->messsage << endl;
		}
		

	}

	return 0;
}