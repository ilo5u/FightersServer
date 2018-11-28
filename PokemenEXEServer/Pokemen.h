#pragma once

namespace Pokemen
{
	typedef std::string String;
	typedef std::mutex  Mutex;
	typedef std::thread Thread;

	/// <summary>
	/// 
	/// </summary>
	class Pokemen
	{
	public:
		Pokemen(PokemenType type, int level, Id id = 0);
		Pokemen(const Property& prop, int career);
		Pokemen(const Pokemen& other);
		Pokemen(Pokemen&& other);
		Pokemen& operator=(const Pokemen& other);
		Pokemen& operator=(Pokemen&& other);
		virtual ~Pokemen();

	public:
		// 获取属性值
		int GetId() const;
		PokemenType GetType() const;
		String GetName() const;

		int GetHpoints() const;
		int GetAttack() const;
		int GetDefense() const;
		int GetAgility() const;

		int GetInterval() const;
		int GetCritical() const;
		int GetHitratio() const;
		int GetParryratio() const;
		int GetAnger() const;

		int GetCareer() const;
		int GetLevel() const;
		int GetExp() const;

	public:
		bool Upgrade(int exp);
		bool SetPrimarySkill(int skill);
		bool Promote(int career);
		void RenewProperty(const ::Pokemen::Property& prop, int carrer);

	public:
		void SetMaxHpoints();
		bool InState(BasePlayer::State nowState) const;

	public:
		String Attack(Pokemen& opponent);

	private:
		PBasePlayer m_instance;
	};

	struct BattleMessage
	{
		enum class Type
		{
			DISPLAY,
			RESULT
		};
		Type type;
		String options;

		BattleMessage() = default;
		BattleMessage(BattleMessage::Type type, const String& message);
	};
	typedef std::queue<BattleMessage> BattleMessages;

	class BattleStage
	{
	public:
		BattleStage();
		virtual ~BattleStage();

	public:
		void SetPlayers(const Pokemen& firstPlayer, const Pokemen& secondPlayer);

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
		Pokemen m_firstPlayer;
		Pokemen m_secondPlayer;

		BattleMessages m_messages;
		Mutex          m_messagesMutex;
		HANDLE         m_messagesAvailable;

		HANDLE         m_stateControl;
		Thread         m_battleDriver;

		bool           m_isBattleRunnig;

		char m_battleMessage[BUFLEN];

	private:
		void _RunBattle_();
	};

}