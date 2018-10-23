#pragma once

namespace Pokemen
{
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
		std::string GetName() const;

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
		bool SetName(const std::string& name);

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
		std::string Attack(PokemenManager& opponent);

	private:
		PBasePlayer m_instance;
	};

	struct BattleMessage
	{
		std::string wsOptions;

		BattleMessage() = default;
		BattleMessage(const std::string& message);
	};

	class BattleStage
	{
	public:
		BattleStage();
		virtual ~BattleStage();

	public:
		void AddPlayer(const PokemenManager& first_player, const PokemenManager& second_player);
		void Start();
		void Pause();
		void GoOn();
		void Clear();
		bool IsRunning() const;
		int  GetRoundCnt() const;

		int GetFirstPlayerId() const;
		int GetSecondPlayerId() const;

		BattleMessage ReadMessage();

	private:
		PokemenManager m_first_player;
		PokemenManager m_second_player;
		std::queue<BattleMessage> m_message_queue;
		std::mutex m_message_mutex;
		HANDLE m_message_event;
		HANDLE m_on_off_event;
		std::thread m_battle_thread;

	private:
		int m_round_cnt;

	private:
		bool m_is_battle_on_running;
		void __run_battle__();
	};
}