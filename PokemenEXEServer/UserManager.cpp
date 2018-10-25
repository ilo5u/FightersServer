#include "stdafx.h"
#include "Server.h"
#include "UserManager.h"

#include <thread>

typedef Packet::Type    PacketType;
typedef Server::Message Message;
typedef Message::Type   MessageType;

UserManager::UserManager(const Socket& connectSocket, const SockaddrIn& clientAddr, Server& server) :
	m_connectSocket(connectSocket), m_clientAddr(clientAddr),
	m_sendMutex(), m_sendEvent(nullptr), m_sendPackets(),
	m_connectionClosed(false), m_recvClosedEvent(nullptr), m_sendClosedEvent(nullptr),
	m_server(server),
	m_battleType(), m_readBattleMessageThread(), m_isReadBattleMessageClosed(true), m_battleStage(),
	m_username(), m_pokemens()
{
	// 建立通信消息处理线程
	Thread recvThread{ std::bind(&UserManager::_ClientRecvThread_, this) };
	recvThread.detach();

	Thread sendThread{ std::bind(&UserManager::_ClientSendThread_, this) };
	sendThread.detach();

	// 初始化事件进行线程控制
	m_sendEvent       = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_sendClosedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_recvClosedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

UserManager::~UserManager()
{
	// 优先关闭比赛处理的相关线程
	m_isReadBattleMessageClosed = true;
	if (m_readBattleMessageThread.joinable())
		m_readBattleMessageThread.join();
	m_battleStage.Clear();

	// 等待通信模块线程结束 保证资源释放完毕
	WaitForSingleObject(m_recvClosedEvent, INFINITE);
	WaitForSingleObject(m_sendClosedEvent, INFINITE);

	CloseHandle(m_recvClosedEvent);
	CloseHandle(m_sendClosedEvent);
	CloseHandle(m_sendEvent);

	// 关闭连接
	closesocket(m_connectSocket);
}

// 仅由服务器线程调用
void UserManager::InsertAPokemen(const String& info)
{
	size_t pos = info.find_first_of(',');
	String first{ info.substr(0, pos) };	// 包含唯一标识以及名字信息
	String second{ info.substr(pos + 1, info.size() - pos - 1) }; // 包含各项属性值

	// 获取小精灵所有信息
	int  identity;
	char name[BUFSIZ];
	int  type;
	int  hpoints, attack, defense, agility;
	int  brea, critical, hitratio, parryratio;
	int  exp, level;
	sscanf(first.c_str(), "%d\nNAME=%s", &identity, name);
	sscanf(second.c_str(), "TY=%d,HP=%d,AT=%d,DE=%d,AG=%d,BR=%d,CR=%d,HI=%d,PA=%d,EX=%d,LE=%d",
		&type,
		&hpoints, &attack, &defense, &agility,
		&brea, &critical, &hitratio, &parryratio,
		&exp, &level);

	// 创建并初始化小精灵
	Pokemen::PokemenManager pokemen{ static_cast<PokemenType>(type) };
	pokemen.SetID(identity);
	pokemen.SetName(name);

	pokemen.SetHpoints(hpoints);
	pokemen.SetAttack(attack);
	pokemen.SetDefense(defense);
	pokemen.SetAgility(agility);
	pokemen.SetBreak(brea);

	pokemen.SetCritical(critical);
	pokemen.SetHitratio(hitratio);
	pokemen.SetParryratio(parryratio);

	pokemen.SetExp(exp);
	pokemen.SetLevel(level);

	m_pokemens.push_back(pokemen);
}

void UserManager::WritePacket(const Packet& packet)
{
	m_sendMutex.lock();

	m_sendPackets.push(packet);
	SetEvent(m_sendEvent);

	m_sendMutex.unlock();
}

ULONG UserManager::GetUserID() const
{
	return m_clientAddr.sin_addr.S_un.S_addr;
}

void UserManager::SetUsername(const String& name)
{
	m_username = name;
}

std::string UserManager::GetUsername() const
{
	return m_username;
}

void UserManager::_ReadBattleMessageThread_()
{
	Packet packet;
	packet.type = PacketType::BATTLE_MESSAGE;

	String battleMessage;
	while (!m_isReadBattleMessageClosed)
	{
		battleMessage = std::move(m_battleStage.ReadMessage().options.c_str());
		std::memcpy(packet.data, battleMessage.c_str(), battleMessage.size());
		packet.data[battleMessage.size()] = 0x0;

		if (std::strncmp(packet.data, "GAME END", std::strlen("GAME END")) == 0)
			break;

		WritePacket(packet);
	}

	if (std::strncmp(packet.data, "GAME END", std::strlen("GAME END")) == 0)
	{
		Message message;
		message.type = MessageType::HANDLE_BATTLE_RESULT;
		message.data.battleResult.battleType = m_battleType;

		sscanf(packet.data, "GAME END WITH %d", &message.data.battleResult.winner);

		message.data.battleResult.firstId  = m_battleStage.GetFirstPlayerId();
		message.data.battleResult.secondId = m_battleStage.GetSecondPlayerId();
		message.id = m_clientAddr.sin_addr.S_un.S_addr;

		m_server.WriteMessage(message);
	}
}

// constexpr int DEFAULT_BUFFLEN = 4096;
void UserManager::_ClientRecvThread_()
{
	int iRecvBytes = 0;
	PacketType packetType;

	Message message;
	char szRecvBuf[BUFLEN];
	while (!m_connectionClosed)
	{
		// Recive message from client
		ZeroMemory(szRecvBuf, sizeof(szRecvBuf));
		iRecvBytes = recv(m_connectSocket, szRecvBuf, sizeof(szRecvBuf), 0);
		
		message.id = m_clientAddr.sin_addr.S_un.S_addr;
		if (iRecvBytes > 0)
		{
			sscanf(szRecvBuf, "ID=%d\n", &packetType);
			switch (packetType)
			{
			case PacketType::LAUNCH_REQUEST:
			{
				if (m_username.empty())
				{
					message.type = MessageType::USER_LAUNCH;
					sscanf(szRecvBuf, "ID=%d\nNAME=%s\nPASSWORD=%s\n", &packetType,
						message.data.user.name, message.data.user.password);

					printf("\n登陆请求： NAME=%s PASSWORD=%s\n\n",
						message.data.user.name, message.data.user.password);

					m_server.WriteMessage(message);
				}
			}
			break;

			case PacketType::REGISTER_REQUEST:
			{
				message.type = MessageType::USER_REGISTER;
				sscanf(szRecvBuf, "ID=%d\nNAME=%s\nPASSWORD=%s\n", &packetType,
					message.data.user.name, message.data.user.password);

				printf("\n注册请求： NAME=%s PASSWORD=%s\n\n",
					message.data.user.name, message.data.user.password);

				m_server.WriteMessage(message);
			}
			break;

			case PacketType::GET_ONLINE_USERS:
			{
				message.type = MessageType::GET_ONLINE_USERS;
				m_server.WriteMessage(message);
			}
			break;

			case PacketType::BATTLE_REQUEST:
			{
				int battleType = 0;
				sscanf(szRecvBuf, "ID=%d\nTY=%d\n", &packetType, &battleType);
				m_battleStage.Clear();

				printf("收到一个比赛申请。\n");

				if (battleType == 0)
				{
					int pokemenId;
					Property opponent;
					sscanf(szRecvBuf, "ID=%d\nTY=%d\nID=%d\nTY=%d,HP=%d,AT=%d,DE=%d,AG=%d,BR=%d,CR=%d,HI=%d,PA=%d,EX=%d,LE=%d\n",
						&packetType, &m_battleType,
						&pokemenId,
						&opponent.type,
						&opponent.hpoints, &opponent.attack,
						&opponent.defense, &opponent.agility,
						&opponent.brea, &opponent.critical,
						&opponent.hitratio, &opponent.parryratio,
						&opponent.exp, &opponent.level);

					Pokemens::const_iterator it_pokemen;
					if ((it_pokemen = std::find_if(m_pokemens.begin(), m_pokemens.end(), [pokemenId](const Pokemen::PokemenManager& pokemen) {
						return pokemen.GetID() == pokemenId;
					})) != m_pokemens.end())
					{
						// 创建电脑小精灵
						Pokemen::PokemenManager secondPlayer{ opponent.type };
						secondPlayer.SetHpoints(opponent.hpoints);
						secondPlayer.SetAttack(opponent.attack);
						secondPlayer.SetDefense(opponent.defense);
						secondPlayer.SetAgility(opponent.agility);
						secondPlayer.SetBreak(opponent.brea);

						secondPlayer.SetCritical(opponent.critical);
						secondPlayer.SetHitratio(opponent.hitratio);
						secondPlayer.SetParryratio(opponent.parryratio);

						secondPlayer.SetExp(opponent.exp);
						secondPlayer.SetLevel(opponent.level);

						// 启动比赛场
						if (!m_battleStage.IsRunning())
						{
							printf("与电脑的比赛开始。\n");

							m_isReadBattleMessageClosed = true;
							if (m_readBattleMessageThread.joinable())
								m_readBattleMessageThread.join();
							m_battleStage.Clear();

							m_battleStage.SetPlayers(*it_pokemen, secondPlayer);
							m_battleStage.Start();
							m_isReadBattleMessageClosed = false;
							m_readBattleMessageThread = std::move(Thread{ &UserManager::_ReadBattleMessageThread_, this });
						}
					}
				}
				else if (battleType == 1)
				{

				}
			}
			break;

			case PacketType::BATTLE_CONTROL:
			{
				int crt = 0;
				sscanf(szRecvBuf, "ID=%d\nCRT=%d\n", &packetType, &crt);
				if (crt == 0)
				{
					m_battleStage.GoOn();
				}
				else if (crt == 1)
				{
					m_battleStage.Pause();
				}
				else if (crt == 2)
				{
					// 强制停止比赛
					m_isReadBattleMessageClosed = true;
					if (m_readBattleMessageThread.joinable())
						m_readBattleMessageThread.join();
					m_battleStage.Clear();
				}
			}
			break;

			case PacketType::DISCONNECT:
			{
				char szHostName[BUFSIZ];
				sscanf(szRecvBuf, "ID=%d\nHOST=%s\n", &packetType, szHostName);
				printf("\n登出请求： HOST=%s\n\n>", szHostName);

				message.type = MessageType::USER_CLOSED;
				m_server.WriteMessage(message);

				m_connectionClosed = true;
				SetEvent(m_recvClosedEvent);
				return;
			}
			break;

			default:
				break;
			}
		}
		else
		{	// close
			message.type = MessageType::USER_CLOSED;
			m_server.WriteMessage(message);

			m_connectionClosed = true;
			SetEvent(m_recvClosedEvent);
			return;
		}
	}

	SetEvent(m_recvClosedEvent);
}

void UserManager::_ClientSendThread_()
{
	Packet sendPacket;
	int  iSendBytes = 0;
	char szSendBuf[BUFLEN];

	while (!m_connectionClosed)
	{
		WaitForSingleObject(m_sendEvent, 2000);

		bool sendValid = false;
		m_sendMutex.lock();
		ResetEvent(m_sendEvent);

		if (!m_sendPackets.empty())
		{
			sendValid  = true;
			sendPacket = m_sendPackets.front();
			m_sendPackets.pop();
		}

		m_sendMutex.unlock();

		if (sendValid && sendPacket.type != PacketType::INVALID)
		{
			ZeroMemory(szSendBuf, sizeof(szSendBuf));
			sprintf(szSendBuf, "ID=%u\n", sendPacket.type);

			switch (sendPacket.type)
			{
			case PacketType::LAUNCH_SUCCESS:
			case PacketType::LAUNCH_FAILED:
			case PacketType::REGISTER_SUCCESS:
			case PacketType::REGISTER_FAILED:
			case PacketType::DISTRIBUTE_POKEMENS:
				break;

			case PacketType::INSERT_A_POKEMEN:
			case PacketType::UPDATE_USERS:
			case PacketType::SET_ONLINE_USERS:
			case PacketType::BATTLE_MESSAGE:
			case PacketType::BATTLE_RESULT:
			{
				int iLen = (int)std::strlen(szSendBuf);
				std::memcpy(szSendBuf + iLen, sendPacket.data, std::strlen(sendPacket.data));
			}
			break;

			default:
				break;
			}

			iSendBytes = send(m_connectSocket, szSendBuf, (int)std::strlen(szSendBuf) + 1, 0);

			if (iSendBytes == SOCKET_ERROR)
			{
				Message message;
				message.type = MessageType::USER_CLOSED;
				message.id   = m_clientAddr.sin_addr.S_un.S_addr;

				m_server.WriteMessage(message);

				m_connectionClosed = true;
				SetEvent(m_sendClosedEvent);
				return;
			}
		}
	}

	SetEvent(m_sendClosedEvent);
}