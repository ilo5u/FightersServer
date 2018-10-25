#pragma once

namespace Pokemen
{
	typedef std::string String;
	typedef std::mutex  Mutex;
	typedef std::thread Thread;

	/// <summary>
	/// 
	/// </summary>
	class PokemenManager
	{
	public:
		PokemenManager(PokemenType type, int level = 1);
		PokemenManager(const PokemenManager& other);
		PokemenManager(PokemenManager&& other);
		PokemenManager& operator=(const PokemenManager& other);
		PokemenManager& operator=(PokemenManager&& other);
		virtual ~PokemenManager();

	public:
		int GetID() const;
		String GetName() const;

		int GetHpoints() const;
		int GetAttack() const;
		int GetDefense() const;
		int GetAgility() const;

		int GetBreak() const;
		int GetCritical() const;
		int GetHitratio() const;
		int GetParryratio() const;
		int GetAnger() const;

		int GetLevel() const;
		int GetExp() const;

	public:
		int SetID(int id);
		bool SetName(const String& name);

		bool SetHpoints(int hpoints);
		bool SetAttack(int attack);
		bool SetDefense(int defense);
		bool SetAgility(int agility);

		bool SetBreak(int brea);
		bool SetCritical(int critical);
		bool SetHitratio(int hitratio);
		bool SetParryratio(int parryratio);
		bool SetAnger(int anger);

		bool SetLevel(int level);
		bool SetExp(int exp);

	public:
		PokemenType GetType() const;
		bool InState(BasePlayer::State nowState) const;

	public:
		String Attack(PokemenManager& opponent);

	private:
		PBasePlayer m_instance;
	};

	struct BattleMessage
	{
		String options;

		BattleMessage() = default;
		BattleMessage(const String& message);
	};
	typedef std::queue<BattleMessage> BattleMessages;

	class BattleStage
	{
	public:
		BattleStage();
		virtual ~BattleStage();

	public:
		void SetPlayers(const PokemenManager& firstPlayer, const PokemenManager& secondPlayer);

		void Start();
		bool Pause();
		bool GoOn();
		void Clear();

		bool IsRunning() const;
		int  GetRoundCnt() const;

		int GetFirstPlayerId() const;
		int GetSecondPlayerId() const;

		BattleMessage ReadMessage();

	private:
		int m_roundsCnt;
		PokemenManager m_firstPlayer;
		PokemenManager m_secondPlayer;

		BattleMessages m_messages;
		Mutex          m_messagesMutex;
		HANDLE         m_messagesEvent;

		HANDLE         m_stateEvent;
		Thread         m_battleThread;

		bool           m_isBattleRunnig;

	private:
		void _RunBattle_();
	};
}