#include<winsock2.h>
#include<iostream>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
#define BUF_SIZE 2048
#define HANDLE_SIZE 8 

DWORD WINAPI recvThread(LPVOID param);
DWORD WINAPI sendThread(LPVOID param);


typedef struct User
{
	int type;
	char id[9];
	char password[17];
	sockaddr_in client_addr;
	SOCKET user_socket;
}User;

typedef struct Message
{
	int type;
	char origin_userId[9];
	char des_userId[9];
	char messsage[1024];
}Message;

User onlineUserList[1024];
User userList[1024];
User friendList[1024][1024];
User groupChat[1024];

int userTotal = 0;
int groupTotal = 0;
int total = 0;
int friendTotal = 0;
int num = 0;
int main()
{
	//初始化环境，创建服务器套接字
	WSADATA wsaDate;
	WSAStartup(0x0202, &wsaDate);
	SOCKET sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(9990);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	//绑定服务器套接字，并进行监听
	bind(sSocket, (PSOCKADDR)&serverAddr, sizeof(SOCKADDR_IN));
	listen(sSocket, 1);
	SOCKET cSocket;
	HANDLE threadHandles[HANDLE_SIZE];
	cout << "服务器启动成功！" << endl;

	FILE* fp = fopen("userInfo.txt", "r");
	FILE* fp1 = fopen("friendList.txt", "r");
	if (fp == NULL||fp1==NULL)
	{
		printf("未能找到文件！！");
		return 0;
	}
	else
	{
		char str[100];
		while (fgets(str, 100, fp) != NULL)
		{
			str[strcspn(str, "\n")] = 9;
			char* q = strtok(str, "\t");
			strcpy(userList[userTotal].id, q);
			q = strtok(NULL, "\t");
			strcpy(userList[userTotal].password, q);

			
			userTotal++;
		}

		while (fgets(str, 100, fp1) != NULL)
		{
			str[strcspn(str, "\n")] = 9; // 去除换行符
			int i = 1;
			char* q = strtok(str, "\t");
			
			while (q != NULL)
			{
				strcpy(friendList[friendTotal][i++].id, q);
				q = strtok(NULL, "\t");
			}
			friendList[friendTotal][0].type = i;
			friendTotal++;
		}

		cout << "用户信息加载成功！"<<endl;
	}

	while (1)
	{
		User user;
		sockaddr_in client_addr;
		int addrlen = sizeof(client_addr);
		//接收客户端连接
		cSocket = accept(sSocket, (struct sockaddr*)&client_addr, &addrlen);
		if (cSocket == INVALID_SOCKET)
		{
			cout << "accept failed" << endl;
		}
		else
		{
			cout << "主机IP：" << inet_ntoa(client_addr.sin_addr) << " 端口：" << htons(client_addr.sin_port) << " 的用户正在登录！" << endl;
			user.client_addr = client_addr;
			user.user_socket = cSocket;
			threadHandles[0] = CreateThread(NULL, 0, recvThread, &user, 0, NULL);
		}
	}
	//WaitForMultipleObjects(1, threadHandles, true, INFINITE);
	closesocket(sSocket);
	closesocket(cSocket);
	WSACleanup();
	system("pause");
	return 0;
}


char recvBuf[BUF_SIZE];
char sendBuf[BUF_SIZE];
DWORD WINAPI recvThread(LPVOID param)//接收信息线程
{
	User user = *(User*)param;
	int index;
	int f = 0;
	while (1)
	{
		ZeroMemory(recvBuf, BUF_SIZE);
		ZeroMemory(sendBuf, BUF_SIZE);
		if (f == 0)
		{
			int retVal = recv(user.user_socket, recvBuf, BUF_SIZE, 0);
			User* user1 = (User*)recvBuf;
			
			if (retVal == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAECONNRESET)
				{
					cout << "主机IP：" << inet_ntoa(user.client_addr.sin_addr) << " 端口：" << htons(user.client_addr.sin_port) << " 的用户放弃登录" << endl;
				}
				break;
			}
			else
			{
				int islogin = 0;
				for (int i = 0; i < total; i++)
				{
					if (strcmp(onlineUserList[i].id, user1->id) == 0)
					{
						islogin = 1;
					}
				}
				int isfound = 0,isfound1=0;
				for (int i = 0; i < userTotal; i++)
				{
					if (strcmp(userList[i].id, user1->id) == 0)
					{
						isfound1 = 1;
						if (strcmp(userList[i].password, user1->password) == 0)
						{
							isfound = 1;
							break;
						}
					}
				}
				if (user1->type==1)
				{

					if (isfound == 1)
					{
						if (islogin == 1)
						{
							Message message1;
							memset(&message1, 0, sizeof(Message));
							message1.type = -1;
							strcpy(message1.messsage, "该用户已经登录");
							memcpy(sendBuf, &message1, sizeof(Message));
							int res = send(user.user_socket, sendBuf, sizeof(Message), 0);
						}
						else
						{
							User tmp;
							tmp.user_socket = user.user_socket;
							strcpy(tmp.id, user1->id); strcpy(tmp.password, user1->password);
							onlineUserList[total++] = tmp;
							strcpy(user.id, user1->id);
							printf("主机IP：%s 端口：%d的用户%s已经上线！当前服务器在线用户：%d\n", inet_ntoa(user.client_addr.sin_addr), htons(user.client_addr.sin_port), user1->id, total);
							char buf2[sizeof(Message)];
							Message message1;
							memset(&message1, 0, sizeof(Message));
							message1.type = -1;
							strcpy(message1.messsage, "登录成功");
							memcpy(buf2, &message1, sizeof(Message));
							int res = send(user.user_socket, buf2, sizeof(Message), 0);
							f = 1;
						}

					}
					else
					{
						Message message1;
						memset(&message1, 0, sizeof(Message));
						message1.type = -1;
						strcpy(message1.messsage, "用户名或密码错误");
						memcpy(sendBuf, &message1, sizeof(Message));
						int res = send(user.user_socket, sendBuf, sizeof(Message), 0);
						f = 0;
					}
				}
				else if(user1->type==2)
				{
					if (isfound1 == 1)
					{
						Message message1;
						memset(&message1, 0, sizeof(Message));
						message1.type = -1;
						strcpy(message1.messsage, "用户名已存在!!");
						memcpy(sendBuf, &message1, sizeof(Message));
						int res = send(user.user_socket, sendBuf, sizeof(Message), 0);
					}
					else
					{
						FILE* file = fopen("userInfo.txt", "a");
						FILE* fp = fopen("friendList.txt", "a");
						if (fp == NULL||file==NULL)
						{
							cout << "文件未找到" << endl;
						}

						else
						{
							char str[100] = "";
							strcat(str, user1->id);strcat(str, "\t");
							strcat(str, user1->password);strcat(str, "\n");
							fprintf(file, str);
							fclose(file);
							strcpy(userList[userTotal].id, user1->id);
							strcpy(userList[userTotal].password, user1->password);
							userTotal++;
							Message message1;
							memset(&message1, 0, sizeof(Message));
							message1.type = -1;
							strcpy(message1.messsage, "注册成功!!");
							memcpy(sendBuf, &message1, sizeof(Message));
							int res = send(user.user_socket, sendBuf, sizeof(Message), 0);
							friendList[friendTotal][0].type = 2;
							strcpy(friendList[friendTotal][1].id,user1->id);
							friendTotal++;
							char str1[100] = "";
							strcat(str1, user1->id);
							strcat(str1, "\n");
							fprintf(fp, str1);
							fclose(fp);
						}
						
					}
				}

				
			}
			
		}
		else
		{
			ZeroMemory(recvBuf, BUF_SIZE);
			int retVal = recv(user.user_socket, recvBuf, BUF_SIZE, 0);
			if (retVal == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAECONNRESET)//对方通信已关闭
				{
					if (total == 1)
					{
						total--;
					}
					else
					{
						for (int i = 0; i < total; i++)
						{
							if (strcmp(onlineUserList[i].id, user.id) == 0)
							{
								onlineUserList[i] = onlineUserList[total - 1];
								total--;
							}
						}
					}
					printf("主机IP：%s 端口：%d的用户%s已经下线！当前服务器在线用户：%d\n", inet_ntoa(user.client_addr.sin_addr), htons(user.client_addr.sin_port), user.id, total);
					break;
				}
			}
			char buf1[sizeof(Message)];
			memcpy(buf1, recvBuf, sizeof(Message));
			Message* message = (Message*)recvBuf;
			
			if (message->type == 10)
			{
				int f = 0;
				for (int i = 0; i < friendTotal; i++)
				{
					if (strcmp(friendList[i][1].id, user.id) == 0)
					{
						index = i;
						f = 1;
						break;
					}
				}
				if (f == 1)
				{
					User friends[1024];
					memset(friends, 0, sizeof(User)*1024);
					int j = 0;
					for (int i = 0; i < friendList[index][0].type; i++)
					{
						friends[j++] = friendList[index][i];
					}
					Message message1;
					memset(&message1, 0, sizeof(Message));
					message1.type = 10;
					memcpy(message1.messsage, friends, sizeof(User)*j);
					ZeroMemory(sendBuf, BUF_SIZE);
					memcpy(sendBuf, &message1, sizeof(Message));
					send(user.user_socket, sendBuf, sizeof(Message), 0);
				}
				

			}
			else if(message->type==20)
			{
				int isfound = 0;
				for (int i = 0; i < userTotal; i++)
				{
					if (strcmp(userList[i].id, message->des_userId) == 0)
					{
							isfound = 1;
							break;
					}
				}
				int isfriend = 0;
				for (int i = 2; i < friendList[index][0].type; i++)
				{
					if (strcmp(friendList[index][i].id, message->des_userId) == 0)
					{
						isfriend = 1;
					}
				}
				if (isfound == 1)
				{
					if (isfriend == 1)
					{
						Message message1;
						memset(&message1, 0, sizeof(Message));
						message1.type = -1;
						strcpy(message1.des_userId, message->des_userId);
						strcpy(message1.messsage, "对方已经是你的好友！！不可重复添加");
						memcpy(sendBuf, &message1, sizeof(Message));
						send(user.user_socket, sendBuf, sizeof(Message), 0);
					}
					else
					{
						strcpy(friendList[index][friendList[index][0].type++].id, message->des_userId);
						int index1;
						for (int i = 0; i < friendTotal; i++)
						{
							if (strcmp(friendList[i][1].id, message->des_userId) == 0)
							{
								index1 = i;
								break;
							}
						}
						strcpy(friendList[index1][friendList[index1][0].type++].id, user.id);
						Message message1;
						memset(&message1, 0, sizeof(Message));
						message1.type = -1;
						strcpy(message1.des_userId, message->des_userId);
						strcpy(message1.messsage, "添加成功");
						memcpy(sendBuf, &message1, sizeof(Message));
						send(user.user_socket, sendBuf, sizeof(Message), 0);

						int isOnline = 0;
						SOCKET desSocket;
						for (int i = 0; i < total; i++)
						{
							if (strcmp(message->des_userId, onlineUserList[i].id) == 0)
							{
								isOnline = 1;
								desSocket = onlineUserList[i].user_socket;
								break;
							}
						}
						if (isOnline == 1)
						{
							memset(&message1, 0, sizeof(Message));
							message1.type = 1;
							strcpy(message1.des_userId, message->des_userId);
							strcpy(message1.origin_userId, user.id);
							strcpy(message1.messsage, "已经添加你为好友！！");
							memcpy(sendBuf, &message1, sizeof(Message));
							send(desSocket, sendBuf, sizeof(Message), 0);

							memset(&message1, 0, sizeof(Message));
							message1.type = -1;
							strcpy(message1.des_userId, user.id);
							strcpy(message1.messsage, "添加成功");
							memcpy(sendBuf, &message1, sizeof(Message));
							send(desSocket, sendBuf, sizeof(Message), 0);
						}
					}


				}
				else
				{
					Message message1;
					memset(&message1, 0, sizeof(Message));
					message1.type = -1;
					strcpy(message1.messsage, "没有该用户！");
					memcpy(sendBuf, &message1, sizeof(Message));
					send(user.user_socket, sendBuf, sizeof(Message), 0);
				}
			}
			else if (message->type == 99)
			{
				for (int i = 0; i < groupTotal; i++)
				{
					Message message1;
					memset(&message1, 0, sizeof(Message));
					message1.type = 0;
					
					strcpy(message1.messsage, "加入聊天室");
					strcpy(message1.origin_userId, user.id);
					memcpy(sendBuf, &message1, sizeof(Message));
					send(groupChat[i].user_socket, sendBuf, sizeof(Message), 0);
				}
				strcpy(groupChat[groupTotal].id, message->origin_userId);
				groupChat[groupTotal++].user_socket = user.user_socket;

			}
			else if (message->type == 100)
			{
				if (strcmp(message->messsage, "quit") == 0)
				{
					for (int i = 0; i < groupTotal; i++)
					{
						if (strcmp(groupChat[i].id, user.id) == 0)
						{
							groupChat[i] = groupChat[groupTotal - 1];
							groupTotal--;
						}
					}
					strcpy(message->messsage, "退出聊天室");
				}
				for (int i = 0; i < groupTotal; i++)
				{
					memcpy(buf1, message, sizeof(Message));
					send(groupChat[i].user_socket, buf1, sizeof(Message), 0);
				}
			}
			else
			{
				int isOnline = 0;
				SOCKET desSocket;
				for (int i = 0; i < total; i++)
				{
					if (strcmp(message->des_userId, onlineUserList[i].id) == 0)
					{
						isOnline = 1;
						desSocket = onlineUserList[i].user_socket;
						break;
					}
				}
				if (isOnline == 1)
				{
					cout << "对方在线" << endl;
					if (message->type == 0)
					{
						message->type = -1;
						strcpy(message->messsage, "对方在线");
						memcpy(buf1, message, sizeof(Message));
						send(user.user_socket, buf1, sizeof(Message), 0);
					}
					else
					{
						memcpy(buf1, message, sizeof(Message));
						send(desSocket, buf1, sizeof(Message), 0);
					}

				}
				else
				{
					if (message->type == 0)
					{
						message->type = -1;
						strcpy(message->messsage, "对方不在线");
						memcpy(buf1, message, sizeof(Message));
						send(user.user_socket, buf1, sizeof(Message), 0);
					}
				}
			}


		}
		
	}

	FILE* fp = fopen("friendList.txt", "w");
	for (int i = 0; i < friendTotal; i++)
	{
		char str[256];
		strcpy(str, "");
		for (int j = 1; j < friendList[i][0].type-1; j++)
		{
			strcat(str, friendList[i][j].id);
			strcat(str, "\t");
		}
		strcat(str, friendList[i][friendList[i][0].type - 1].id);
		strcat(str, "\n");
		fprintf(fp, str);
	}
	fclose(fp);
	return 0;
}


