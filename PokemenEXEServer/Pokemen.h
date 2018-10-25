#pragma once

/// <summary>
/// 
/// </summary>
namespace Pokemen
{
	/// <summary>
	/// 
	/// </summary>
	enum class PokemenType
	{
		DEFAULT,
		TANK,
		LION,
		HEDGEHOG,
		EAGLE
	};

	typedef int16_t             Value;
	typedef std::string         String;
	typedef std::vector<String> Strings;

	/// <summary>
	/// 
	/// </summary>
	class PokemenProperty
	{
	public:
		PokemenProperty(String name, Value hitpoints, Value attack, Value defense, Value agility, PokemenType type);
		PokemenProperty(const PokemenProperty&) = default;
		virtual ~PokemenProperty();

		PokemenProperty& operator=(const PokemenProperty&) = delete;

	public:
		// 获取属性值
		int GetID() const;
		String GetName() const;

		Value GetHpoints() const;
		Value GetAttack() const;
		Value GetDefense() const;
		Value GetAgility() const;

		Value GetBreak() const;
		Value GetCritical() const;
		Value GetHitratio() const;
		Value GetParryratio() const;

		PokemenType GetType() const;

		Value GetExp() const;
		Value GetLevel() const;

	public:
		// 设置属性值
		bool SetID(int id);
		bool SetName(const String& name);

		bool SetHpoints(Value hpoints);
		bool SetAttack(Value attack);
		bool SetDefense(Value defense);
		bool SetAgility(Value agility);

		bool SetBreak(Value brea);
		bool SetCritical(Value critical);
		bool SetHitratio(Value hitratio);
		bool SetParryratio(Value parryratio);

		bool SetExp(Value exp);
		bool SetLevel(Value level);

	protected:
		/// <summary>
		/// 
		/// </summary>
		static Value BreakValueCalculator(Value base, Value primary_affect, Value secondary_affect);

		/// <summary>
		/// 
		/// </summary>
		static Value CriticalValueCalculator(Value base, Value affect);

		/// <summary>
		/// 
		/// </summary>
		static Value HitratioValueCalculator(Value base, Value primary_affect, Value secondary_affect);

		/// <summary>
		/// 
		/// </summary>
		static Value ParryratioValueCalculator(Value base, Value primary_affect, Value secondary_affect);

		/// <summary>
		/// 
		/// </summary>
		static Value AttackDamageCalculator(Value primary_affect, Value secondary_affect);

		/// <summary>
		/// 
		/// </summary>
		static Value HealingHpointsCalculator(Value primary_affect, Value secondary_affect);

		/// <summary>
		/// 
		/// </summary>
		static Value BloodingDamageCalculator(Value primary_affect, Value secondary_affect);

	protected:
		// 唯一标识
		int         m_id;
		PokemenType m_type;
		String      m_name;

		// 属性
		Value m_hpoints;
		Value m_attack;
		Value m_defense;
		Value m_agility;

		Value m_break;
		Value m_critical;
		Value m_hitratio;
		Value m_parryratio;

		Value m_exp;
		Value m_level;

	protected:
		// common initializing data part
		static const Value propertyOffset = 4;

		static const Value breakBase      = 1000;
		static const Value criticalBase   = 20;
		static const Value hitratioBase   = 90;
		static const Value parryratioBase = 10;

		static const Value angerIncrementBase = 10;
		static const Value bloodingDamageBase = 20;

		static const Value expBase            = 1000;
		static const Value hpointsLevelupBase = 100;
		static const Value levelupPropertiesIncrementBase = 21;

		static const Value bloodingRoundsBase = 5;
		static const Value weakenRoundsBase   = 3;
		static const Value dizzyRoundsBase    = 4;
		static const Value sounderRoundsBase  = 4;
		static const Value silentRoundsBase   = 2;

		// common updating data part
		static const Value angerLimitation = 100;
		static const Value levelLimitation = 15;
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
			NORMAL     = 0x0001,
			ANGRIED    = 0x0002,
			DBANGRY    = 0x0004,
			WEAKEN     = 0x0008,

			SUNDERED   = 0x0010,
			DIZZYING   = 0x0020,

			SILENT     = 0x0040,
			REBOUNDING = 0x0080,

			BLOODING   = 0x0100,
			SLOWED     = 0x0200,
			DEAD       = 0x0400,
			MASK       = 0xFFFF
		};

		struct SkillBase { };

	public:
		bool InState(State inState) const;
		bool AddState(State newState);
		bool SubState(State oldState);

	public:
		BasePlayer(String name, Value hitpoints, Value attack, Value defense, Value agility, PokemenType type);
		virtual ~BasePlayer();

	public:
		typedef BasePlayer * HCPokemenPlayer;
		virtual String Attack(HCPokemenPlayer oppoent);
		virtual Value  IsAttacked(Value damage);
		virtual bool   SetPrimarySkill(const SkillBase& skill);

	public:
		bool SetAnger(int anger);
		bool Update(int extra);

	public:
		State GetState() const;
		Value GetAnger() const;

	protected:
		State m_state;
		Value m_anger;
		Value m_hpointsLimitation;
		Value m_attackOriginal;
		Value m_defenseOriginal;
		Value m_agilityOriginal;

	protected:
		Value m_dizzyRoundsCnt;
		Value m_bloodingRoundsCnt;
		Value m_weakenRoundsCnt;
		Value m_sunderedRoundsCnt;
		Value m_slowedRoundsCnt;
		Value m_silentRoundsCnt;

	protected:
		virtual void _LevelUpPropertiesDistributor_();
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
			Type  m_primarySkill;
			Value m_doubleAngryProbability = 10;
			Value m_selfHealingProbability = 30;

			Value m_selfHealingIndex = 20;

			static const Type  mainSkill     = Type::REBIRTH;
			             Value m_weakenIndex = 50;

			Skill(Type primarySkill);
		};

	public:
		Tank(Skill::Type primarySkill, int level = 1);
		virtual ~Tank();

	public:
		String Attack(HCPokemenPlayer opponent);
		Value  IsAttacked(Value damage);
		bool   SetPrimarySkill(Skill::Type primarySkill);

	private:
		Skill m_skill;
		Value m_angriedCnt = 1;

	private:
		void _LevelUpPropertiesDistributor_();

	private:
		static const Value hitpointsBase = 480;
		static const Value attackBase    = 36;
		static const Value defenseBase   = 38;
		static const Value agilityBase   = 24;
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
			Type  m_primarySkill;
			Value m_sunderArmProbability = 20;
			Value m_makeDizzyProbability = 40;

			Value m_sunderArmIndex = 50;

			static const Type mainSkill          = Type::AVATAR;
			             Value m_strengthenIndex = 200;

			Skill(Type primarySkill);
		};

	public:
		Lion(Skill::Type primarySkill, int level = 1);
		virtual ~Lion();

	public:
		String Attack(BasePlayer& opponent);
		Value  IsAttacked(Value damage);
		bool   SetPrimarySkill(Skill::Type skill);

	private:
		Skill m_skill;

	private:
		static const Value	hitpointsBase = 360;
		static const Value	attackBase    = 60;
		static const Value	defenseBase   = 48;
		static const Value	agilityBase   = 36;
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
			Type  m_primarySkill;
			Value m_sunkInSilenceProbability = 10;
			Value m_reboundDamageProbability = 60;

			Value m_reboundDamageIndex = 30;

			static const Type mainSkill = Type::ARMOR;
			Value m_defenseIndex = 400;

			Skill(Type primarySkill);
		};

	public:
		Hedgehog(Skill::Type primarySkill, int level = 1);
		virtual ~Hedgehog();

	public:
		String Attack(BasePlayer& opponent);
		Value  IsAttacked(Value damage);
		bool   SetPrimarySkill(Skill::Type primarySkill);

	private:
		Skill m_skill;

	private:
		static const Value	hitpointsBase = 420;
		static const Value	attackBase    = 36;
		static const Value	defenseBase   = 60;
		static const Value	agilityBase   = 24;
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
			Type  m_primarySkill;
			Value m_tearingProbability = 30;
			Value m_slowProbability    = 10;

			Value m_slowIndex = 200;

			static const Type mainSkill = Type::INSPIRE;
			Value m_fastenIndex = 50;
			Value m_stolenIndex = 50;

			Skill(Type primarySkill);
		};

	public:
		Eagle(Skill::Type primarySkill, int level = 1);
		virtual ~Eagle();

	public:
		String Attack(BasePlayer& opponent);
		Value  IsAttacked(Value damage);
		bool   SetPrimarySkill(Skill::Type primarySkill);

	private:
		Skill m_skill;

	private:
		static const Value hitpointsBase = 400;
		static const Value attackBase    = 48;
		static const Value defenseBase   = 24;
		static const Value agilityBase   = 60;
	};
	typedef Eagle * PEaglePlayer;
}