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
		void AddPlayer(const PokemenManager& player_1, const PokemenManager& player_2);
		void Start();
		void Pause();
		void GoOn();

		BattleMessage ReadMessage();

		PokemenManager GetPlayer_1();
		PokemenManager GetPlayer_2();

	private:
		PokemenManager m_player_1;
		PokemenManager m_player_2;
		std::queue<BattleMessage> m_message_queue;
		std::mutex m_message_mutex;
		HANDLE m_message_event;
		HANDLE m_on_off_event;

		void __run_battle__();
	};
}