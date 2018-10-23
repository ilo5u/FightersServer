#pragma once

/// <summary>
/// 
/// </summary>
namespace Pokemen
{
	enum class PokemenType
	{
		DEFAULT,
		TANK,
		LION,
		HEDGEHOG,
		EAGLE
	};

	/// <summary>
	/// 
	/// </summary>
	class PokemenProperty
	{
	public:
		PokemenProperty(std::string name, int16_t hitpoints, int16_t attack, int16_t defense, int16_t agility, PokemenType type);
		PokemenProperty(const PokemenProperty&) = default;
		virtual ~PokemenProperty();

		PokemenProperty& operator=(const PokemenProperty&) = delete;

	public:
		int GetID() const;
		std::string GetName() const;

		int16_t GetHpoints() const;
		int16_t GetAttack() const;
		int16_t GetDefense() const;
		int16_t GetAgility() const;

		int16_t GetBreak() const;
		int16_t GetCritical() const;
		int16_t GetHitratio() const;
		int16_t GetParryratio() const;

		PokemenType GetType() const;

		int16_t GetLevel() const;
		int16_t GetExp() const;

	public:
		bool SetID(int id);
		bool SetName(const std::string& name);

		bool SetHpoints(int16_t hpoints);
		bool SetAttack(int16_t attack);
		bool SetDefense(int16_t defense);
		bool SetAgility(int16_t agility);

		bool SetBreak(int16_t brea);
		bool SetCritical(int16_t critical);
		bool SetHitratio(int16_t hitratio);
		bool SetParryratio(int16_t parryratio);

		bool SetLevel(int16_t level);
		bool SetExp(int16_t exp);

	protected:
		/// <summary>
		/// 
		/// </summary>
		static int16_t BreakValueCalculator(int16_t base, int16_t primary_affect, int16_t secondary_affect);

		/// <summary>
		/// 
		/// </summary>
		static int16_t CriticalValueCalculator(int16_t base, int16_t affect);

		/// <summary>
		/// 
		/// </summary>
		static int16_t HitratioValueCalculator(int16_t base, int16_t primary_affect, int16_t secondary_affect);

		/// <summary>
		/// 
		/// </summary>
		static int16_t ParryratioValueCalculator(int16_t base, int16_t primary_affect, int16_t secondary_affect);

		/// <summary>
		/// 
		/// </summary>
		static int16_t AttackDamageCalculator(int16_t primary_affect, int16_t secondary_affect);

		/// <summary>
		/// 
		/// </summary>
		static int16_t HealingHpointsCalculator(int16_t primary_affect, int16_t secondary_affect);

		/// <summary>
		/// 
		/// </summary>
		static int16_t BloodingDamageCalculator(int16_t primary_affect, int16_t secondary_affect);

	protected:
		int m_id;
		std::string m_name;

		int16_t	m_hpoints;
		int16_t	m_attack;
		int16_t	m_defense;
		int16_t	m_agility;

		int16_t	m_break;
		int16_t	m_critical;
		int16_t	m_hitratio;
		int16_t	m_parryratio;

		PokemenType m_type;

		int16_t m_level;
		int16_t m_exp;

	protected:
		// common initializing data part
		static const int16_t property_offset = 4;

		static const int16_t break_base = 1000;
		static const int16_t critical_base = 20;
		static const int16_t hitratio_base = 90;
		static const int16_t parryratio_base = 10;
		static const int16_t anger_limitation_base = 100;

		static const int16_t anger_increase_base = 10;

		static const int16_t blooding_damage_base = 20;

		static const int16_t blooding_circles_base = 3;
		static const int16_t weaken_circles_base = 3;
		static const int16_t dizzy_circles_base = 3;
		static const int16_t sounder_circles_base = 3;

		// common updating data part
		static const int16_t level_limitation = 15;
		static const int16_t exp_base = 1000;
		static const int16_t hpoints_levelup_base = 100;
		static const int16_t levelup_property_increased = 21;
	};

	/// <summary>
	/// 
	/// </summary>
	class BasePlayer : public PokemenProperty
	{
	public:
		/// <summary>
		/// 
		/// </summary>
		enum class State
		{
			NORMAL = 0x0001,
			ANGRIED = 0x0002,
			DBANGRY = 0x0004,
			WEAKEN = 0x0008,

			SUNDERED = 0x0010,
			DIZZYING = 0x0020,

			SILENT = 0x0040,
			REBOUNDING = 0x0080,

			BLOODING = 0x0100,
			SLOWED = 0x0200,
			DEAD = 0x0400,
			MASK = 0xFFFF
		};

		struct SkillBase { };

	public:
		bool InState(BasePlayer::State inState) const;
		bool AddState(BasePlayer::State newState);
		bool SubState(BasePlayer::State oldState);

	public:
		BasePlayer(std::string name, int16_t hitpoints, int16_t attack, int16_t defense, int16_t agility, PokemenType type);
		virtual ~BasePlayer();

	public:
		typedef BasePlayer * HCPokemenPlayer;
		virtual std::string Attack(HCPokemenPlayer oppoent);
		virtual int16_t IsAttacked(int16_t damage);
		virtual bool SetPrimarySkill(const SkillBase& skill);

	public:
		bool SetAnger(int anger);
		bool Update(int extra);

	public:
		State GetState() const;
		int16_t GetAnger() const;

	protected:
		State	m_state;
		int16_t	m_anger;
		int16_t m_hpoints_limitation;
		int16_t m_attack_original;
		int16_t m_defense_original;
		int16_t m_agility_original;

	protected:
		int16_t m_weaken_circles_cnt;
		int16_t m_blooding_circles_cnt;
		int16_t m_sundered_circles_cnt;
		int16_t m_slowed_circles_cnt;
		int16_t m_silent_circles_cnt;

	protected:
		virtual void _level_up_property_distribution();
	};
	typedef BasePlayer * PBasePlayer;

	/// <summary>
	/// 
	/// </summary>
	class Tank : public BasePlayer
	{
	public:
		struct Skill : public SkillBase
		{
			enum class Type
			{
				DOUBLE_ANGRY,
				SELF_HEALING,
				REBIRTH
			};
			Type    m_primary_skill;
			int16_t m_double_angry_probability = 10;
			int16_t m_self_healing_probability = 30;

			int16_t m_self_healing_index = 20;

			static const Type main_skill = Type::REBIRTH;
			int16_t m_weaken_index = 50;

			Skill(Type primary_skill);
		};

	public:
		Tank(Skill::Type primary_skill, int level = 1);
		virtual ~Tank();

	public:
		std::string Attack(HCPokemenPlayer opponent);
		int16_t     IsAttacked(int16_t damage);
		bool        SetPrimarySkill(Skill::Type primary_skill);

	private:
		Skill   m_skill;
		int16_t m_angried_cnt = 1;

	private:
		void _level_up_property_distribution();

	private:
		static const int16_t hitpoints_base = 480;
		static const int16_t attack_base    = 36;
		static const int16_t defense_base   = 38;
		static const int16_t agility_base   = 24;
	};
	typedef Tank * PTankPlayer;

	/// <summary>
	/// 
	/// </summary>
	class Lion : public BasePlayer
	{
	public:
		struct Skill : public SkillBase
		{
			enum class Type
			{
				SUNDER_ARM,
				MAKE_DIZZY,
				AVATAR
			};
			Type    m_primary_skill;
			int16_t m_sunder_arm_probability = 20;
			int16_t m_make_dizzy_probability = 40;

			int16_t m_sunder_arm_index = 50;

			static const Type main_skill = Type::AVATAR;
			int16_t m_strengthen_index = 200;

			Skill(Type primary_skill);
		};

	public:
		Lion(Skill::Type primary_skill, int level = 1);
		virtual ~Lion();

	public:
		std::string Attack(BasePlayer& opponent);
		int16_t IsAttacked(int16_t damage);
		bool SetPrimarySkill(Skill::Type skill);

	private:
		Skill m_skill;

	private:
		static const int16_t	hitpoints_base = 360;
		static const int16_t	attack_base    = 60;
		static const int16_t	defense_base   = 48;
		static const int16_t	agility_base   = 36;
	};
	typedef Lion * PLionPlayer;

	/// <summary>
	/// 
	/// </summary>
	class Hedgehog : public BasePlayer
	{
	public:
		struct Skill : public SkillBase
		{
			enum class Type
			{
				SUNK_IN_SILENCE,
				REBOUND_DAMAGE,
				ARMOR
			};
			Type    m_primary_skill;
			int16_t m_sunk_in_silence_probability = 10;
			int16_t m_rebound_damage_probability = 60;

			int16_t m_rebound_damage_index = 30;

			static const Type main_skill = Type::ARMOR;
			int16_t m_defensen_index = 400;

			Skill(Type primary_skill);
		};

	public:
		Hedgehog(Skill::Type primary_skill, int level = 1);
		virtual ~Hedgehog();

	public:
		std::string Attack(BasePlayer& opponent);
		int16_t IsAttacked(int16_t damage);
		bool SetPrimarySkill(Skill::Type primary_skill);

	private:
		Skill m_skill;

	private:
		static const int16_t	hitpoints_base = 420;
		static const int16_t	attack_base    = 36;
		static const int16_t	defense_base   = 60;
		static const int16_t	agility_base   = 24;
	};
	typedef Hedgehog * PHedgehogPlayer;

	/// <summary>
	/// 
	/// </summary>
	class Eagle : public BasePlayer
	{
	public:
		struct Skill : public SkillBase
		{
			enum class Type
			{
				TEARING,
				SLOW,
				INSPIRE
			};
			Type    m_primary_skill;
			int16_t m_tearing_probability = 30;
			int16_t m_slow_probability = 10;

			int16_t m_slow_index = 200;

			static const Type main_skill = Type::INSPIRE;
			int16_t m_fasten_index = 50;
			int16_t m_stolen_index = 50;

			Skill(Type primary_skill);
		};

	public:
		Eagle(Skill::Type primary_skill, int level = 1);
		virtual ~Eagle();

	public:
		std::string Attack(BasePlayer& opponent);
		int16_t IsAttacked(int16_t damage);
		bool SetPrimarySkill(Skill::Type primary_skill);

	private:
		Skill m_skill;

	private:
		static const int16_t hitpoints_base = 400;
		static const int16_t attack_base    = 48;
		static const int16_t defense_base   = 24;
		static const int16_t agility_base   = 60;
	};
	typedef Eagle * PEaglePlayer;
}