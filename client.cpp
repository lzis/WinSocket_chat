#include<WinSock2.h>
#include<iostream>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
#define BUF_SIZE 2048
#define HANDLE_SIZE 8
DWORD WINAPI recvThread(LPVOID param);//接收信息线程
DWORD WINAPI sendThread(LPVOID param);//发送信息线程


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
	//初始化环境，创建客户端套接字
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
		cout << "服务端异常！连接失败" << endl;
		closesocket(cSocket);
		WSACleanup();
		return -1;
	}
	User user;
	memset(&user, 0, sizeof(User));
	user.user_socket = cSocket;
	HANDLE threadHandles[HANDLE_SIZE];
	threadHandles[0] = CreateThread(NULL, 0, recvThread, &user, 0, NULL);//将套接字地址作为参数，传递给线程

two:
	cout << "==========welcome to chat !!============" << endl;
	cout << "请输入你的选择：" << endl;
	cout << "1 登录 2 注册 " << endl;
	int op; cin >> op;

	if (op == 1)
	{
	one:
		system("cls");
		cout << "============登录=============" << endl;
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
			cout << "===========欢迎用户【" << user.id << "】登录==============" << endl;
		}
	}
	else if (op == 2)//注册
	{
		cout << "===========注册============" << endl;
		cout << "请输入你的用户id(不超过8位)：" << endl;
		char id1[9]; cin >> id1;
		cout << "请输入你的密码(不超过16位）：" << endl;
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
		cout << "输入有误" << endl;
		goto two;
	}
	

	threadHandles[1] = CreateThread(NULL, 0, sendThread, &user, 0, NULL);//将套接字地址作为参数，传递给线程
	WaitForMultipleObjects(2, threadHandles, true, INFINITE);//等待线程结束
	closesocket(cSocket);
	WSACleanup();
	return 0;
}
char recvBuf[BUF_SIZE];
int isOnline = 0;

DWORD WINAPI sendThread(LPVOID param)//发送信息线程
{
	User user = *(User*)param;
	char buf[sizeof(Message)];
	char chatUser[9];
	while (true)
	{
		ZeroMemory(buf, sizeof(Message));
		cout << "1选择好友聊天 2 添加好友 3 输入对方ID进行聊天 4 聊天室" << endl;
		int op; cin >> op;
		if (op == 1)
		{
			cout << "你的好友列表：" << endl;
			for (int i = 0; i < friendNum; i++)
			{
				cout <<i<<":" << " 【" << friendList[i].id << "】" << " ";
			}
			cout << endl;
			cout << "请选择好友：" << endl;
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
			cout << "请输入添加好友ID：" << endl;
			cin >> friends.des_userId;
			memcpy(sendBuf, &friends, sizeof(Message));
			send(user.user_socket, sendBuf, sizeof(Message), 0);
			Sleep(2000);

		}
		else if (op == 3)
		{
			Message message;
			memset(&message, 0, sizeof(Message));
			cout << "聊天对象：" << endl;
			cin >> chatUser;
	three:	strcpy(message.des_userId, chatUser);
			message.type = 0;//验证对方是否在线
			memcpy(buf, &message, sizeof(Message));
			send(user.user_socket, buf, sizeof(buf), 0);
			Sleep(2000);
			if (isOnline == 1)
			{
				char chatUserId[9];
				strcpy(chatUserId, message.des_userId);
				system("cls");
				cout << "==================正在和用户【" << message.des_userId << "】进行聊天===================" << endl;
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
			cout << "============聊天室(输入quit退出)===========" << endl;
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
			cout << "输入错误" << endl;
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
			if (WSAGetLastError() == WSAECONNRESET)//对方通信已关闭
			{
				cout << "服务器已关闭，客户端通讯中断" << endl;
				break;
			}
			else if (retVal == 0)//对方通信已关闭
			{
				printf("The Server End closed!\n");
				break;
			}
		}
		Message* message = (Message*)recvBuf;
		if (message->type == -1 )
		{
			cout << message->messsage << endl;
			if (strcmp(message->messsage, "对方在线") == 0)
			    isOnline = 1;
			if (strcmp(message->messsage, "登录成功") == 0)
				isLogin = 1;
			if (strcmp(message->messsage, "添加成功") == 0)
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
			cout <<"【"<<message->origin_userId<<"】："<< message->messsage << endl;
		}
		

	}

	return 0;
}