#pragma once

/// <summary>
/// 
/// </summary>
namespace Pokemen
{
	enum class PokemenType
	{
		DEFAULT,
		MASTER,
		KNIGHT,
		GUARDIAN,
		ASSASSIN
	};

	typedef int                 Id;
	typedef int32_t             Value;
	typedef std::string         String;
	typedef std::vector<String> Strings;

	struct Property
	{
		Property();
		Property(int type, const char name[],
			int hpoints, int attack, int defense, int agility,
			int interval, int critical, int hitratio, int parryratio,
			int exp, int level,
			int id = 0);
		Property(const Property&) = default;
		Property& operator=(const Property&) = default;
		Property(Property&&) = default;
		Property& operator=(Property&&) = default;

		Id          m_id;
		PokemenType m_type;
		String      m_name;

		Value m_hpoints;
		Value m_attack;
		Value m_defense;
		Value m_agility;

		Value m_interval;
		Value m_critical;
		Value m_hitratio;
		Value m_parryratio;

		Value m_exp;
		Value m_level;
	};

	class BasePlayer
	{
		friend class Master;
		friend class Knight;
		friend class Guardian;
		friend class Assassin;
	public:
		enum class State
		{
			NORMAL = 0x0000,
			ANGRIED = 0x0001,
			DEAD = 0x0002,

			// 减益状态
			BLEED = 0x0004,
			SLOWED = 0x0008,
			WEAKEN = 0x0010,
			SUNDERED = 0x0020,
			DIZZYING = 0x0040,
			SILENT = 0x0080,

			// 增益状态
			RAGED = 0x0100,
			REBOUND = 0x0200,
			AVATAR = 0x0400,
			INSPIRED = 0x0800,
			ARMOR = 0x1000,

			// 掩码（备用）
			MASK = 0xFFFF
		};
		bool InState(State inState) const;

	public:
		BasePlayer(PokemenType type, String name,
			Value hitpoints, Value attack, Value defense, Value agility,
			Value interval, Value critical, Value hitratio, Value parryratio,
			Id id = 0);
		BasePlayer(const Property& prop);
		BasePlayer(const BasePlayer&) = default;
		BasePlayer& operator=(const BasePlayer&) = default;
		BasePlayer(BasePlayer&&) = default;
		BasePlayer& operator=(BasePlayer&&) = default;
		virtual ~BasePlayer();

	public:
		virtual String Attack(BasePlayer&) = 0;
		virtual Value  IsAttacked(Value) = 0;

	public:
		bool SetAnger(int anger);
		bool Upgrade(int extra);

	public:
		Id GetId() const;
		PokemenType GetType() const;
		String GetName() const;
		Value GetHpoints() const;
		Value GetAttack() const;
		Value GetDefense() const;
		Value GetAgility() const;
		Value GetInterval() const;
		Value GetCritical() const;
		Value GetHitratio() const;
		Value GetParryratio() const;
		Value GetExp() const;
		Value GetLevel() const;

		void SetProperty(const Property& prop);

		Value GetAnger() const;

	public:
		bool SetDizzyingRounds(Value rounds);
		bool SetBleedRounds(Value rounds);
		bool SetSunderedRounds(Value rounds);
		bool SetSlowedRounds(Value rounds);
		bool SetSilentRounds(Value rounds);
		bool SetWeakenRounds(Value rounds);
		void SetMaxHpoints();

	protected:
		struct SkillBase { };
		struct CareerBase { };

	protected:
		static Value IntervalValueCalculator(Value base, Value primaryAffect, Value secondaryAffect);
		static Value CriticalValueCalculator(Value base, Value affect);
		static Value HitratioValueCalculator(Value base, Value primaryAffect, Value secondaryAffect);
		static Value ParryratioValueCalculator(Value base, Value primaryAffect, Value secondaryAffect);
		static Value AttackDamageCalculator(Value primaryAffect, Value secondaryAffect);
		static Value HealingHpointsCalculator(Value primaryAffect, Value secondaryAffect);
		static Value BloodingDamageCalculator(Value primaryAffect, Value secondaryAffect);
		static Value ConvertValueByPercent(Value base, Value index);

	protected:
		struct CommonBasicValues
		{
			static const Value propertyOffset = 4;

			static const Value interval = 1000;
			static const Value critical = 20;
			static const Value hitratio = 90;
			static const Value parryratio = 10;

			static const Value angerInc = 10;
			static const Value bleedDamage = 20;

			static const Value exp = 100;
			static const Value hpointsInc = 100;
			static const Value levelupPropertiesInc = 21;

			static const Value bleedRounds = 5;
			static const Value weakenRounds = 3;
			static const Value dizzyingRounds = 2;
			static const Value sunderedRounds = 3;
			static const Value silentRounds = 4;
			static const Value slowedRounds = 5;

			static const Value angerLimitation = 100;
			static const Value levelLimitation = 15;
		};

	protected:
		bool AddState(State newState);
		bool SubState(State oldState);

	protected:
		struct Effects
		{
			struct Effect
			{
				Value hpoints;
				Value attack;
				Value defense;
				Value agility;
				Value interval;
			};
			Effect sundered;
			Effect slowed;
			Effect weaken;
			Effect avatar;
			Effect inspired;
			Effect armor;
		};

		struct StateRoundsCnt
		{
			Value dizzying;
			Value bleed;
			Value sundered;
			Value slowed;
			Value silent;
			Value inspired;
			Value weaken;
			Value avatar;
			Value armor;
		};

	protected:
		Property m_property;
		Value m_anger;
		State m_state;
		Value m_hpointsLimitation;
		Effects m_effects;
		StateRoundsCnt m_stateRoundsCnt;
		char   m_battleMessage[BUFLEN];

	protected:
		virtual void _LevelUpPropertiesDistributor_() = 0;
	};
	typedef BasePlayer * PBasePlayer;

	class Master : public BasePlayer
	{
	public:
		struct Skill : public SkillBase
		{
			enum class Type
			{
				RAGED, // 愤怒
				SELF_HEALING, // 自愈
				REBIRTH // 死者苏生
			};
			Type  primarySkill;
			static const Type mainSkill = Type::REBIRTH;
			// 技能释放概率值
			Value ragedChance;
			Value selfHealingChance;

			// 技能增益
			Value selfHealingIndex;
			Value weakenIndex;

			Skill(Type primarySkill);
		};

		struct Career : public CareerBase
		{
			enum class Type
			{
				Normal,
				GreatMasterOfLight,
				GreatMasterOfDark
			};
			Type type;

			// 光明大法师
			struct Lighter
			{
				const static Value healingIncIndex = +50;
				const static Value selfHealingChanceIncIndex = +20;
				const static Value damageDecIndex = -10;
				const static Value hpointsIncIndex = +20;
				const static Value weakenDecIndex = -50;
			};

			// 黑暗大法师
			struct Darker
			{
				const static Value selfHealingChanceDecIndex = -50;
				const static Value defenseDecIndex = -10;
				const static Value damageIncIndex = +30;
				const static Value angerExtra = CommonBasicValues::angerInc;
				const static Value weakenIncIndex = +50;
			};

			Career(Career::Type type);
		};

	public:
		Master(int level);
		Master(const Property& prop, Career::Type career);
		Master(const Master&) = default;
		Master& operator=(const Master&) = default;
		Master(Master&&) = default;
		Master& operator=(Master&&) = default;
		virtual ~Master();

	public:
		Career::Type GetCareer() const;
		String Attack(BasePlayer& opponent);
		Value  IsAttacked(Value damage);
		bool   SetPrimarySkill(Skill::Type primarySkill);
		bool   Promote(Career::Type career);

	private:
		Skill  m_skill;
		Career m_career;
		Value  m_angriedCnt;

	private:
		struct BasicProperties
		{
			static const Value hitpoints = 960;
			static const Value attack = 36;
			static const Value defense = 40;
			static const Value agility = 24;
		};

	private:
		void _LevelUpPropertiesDistributor_();
	};
	typedef Master * PMaster;

	class Knight : public BasePlayer
	{
	public:
		struct Skill : public SkillBase
		{
			enum class Type
			{
				SUNDER_ARM, // 致残
				MAKE_DIZZY, // 践踏
				AVATAR // 天神下凡
			};
			Type  primarySkill;
			static const Type mainSkill = Type::AVATAR;

			// 技能释放概率值
			Value sunderArmChance;
			Value makeDizzyChance;

			// 技能增益
			Value sunderArmIndex;
			Value avatarIndex;

			Skill(Type primarySkill);
		};

		struct Career : public CareerBase
		{
			enum class Type
			{
				Normal,
				Ares,
				Athena
			};
			Type type;

			// 战神·阿瑞斯
			struct Ares
			{
				const static Value sunderArmChanceIncIndex = +20;
				const static Value sunderArmIncIndex = +10;
				const static Value damageIncIndex = +25;
				const static Value intervalIncIndex = +200;
			};

			// 战争女神·雅典娜
			struct Athena
			{
				const static Value makeDizzyChanceIncIndex = +20;
				const static Value defenseIncIndex = +10;
				const static Value damageDecIndex = -10;
				const static Value intervalDecIndex = -100;
			};

			Career(Career::Type type);
		};

	public:
		Knight(int level);
		Knight(const Property& prop, Career::Type career);
		Knight(const Knight&) = default;
		Knight& operator=(const Knight&) = default;
		Knight(Knight&&) = default;
		Knight& operator=(Knight&&) = default;
		virtual ~Knight();

	public:
		Career::Type GetCareer() const;
		String Attack(BasePlayer& opponent);
		Value  IsAttacked(Value damage);
		bool   SetPrimarySkill(Skill::Type skill);
		bool   Promote(Career::Type career);

	private:
		Skill  m_skill;
		Career m_career;

	private:
		struct BasicProperties
		{
			static const Value	hitpoints = 640;
			static const Value	attack = 50;
			static const Value	defense = 36;
			static const Value	agility = 36;
			static const Value  avatarRounds = 3;
		};

	private:
		void _LevelUpPropertiesDistributor_();
	};
	typedef Knight * PKnight;

	class Guardian : public BasePlayer
	{
	public:
		struct Skill : public SkillBase
		{
			enum class Type
			{
				SUNK_IN_SILENCE, // 沉默
				REBOUND_DAMAGE, // 背刺
				ARMOR // 全副武装
			};
			Type  primarySkill;
			Value sunkInSilenceChance;
			Value reboundDamageChance;

			Value reboundDamageIndex;

			static const Type mainSkill = Type::ARMOR;
			Value defenseIndex;

			Skill(Type primarySkill);
		};

		struct Career : public CareerBase
		{
			enum class Type
			{
				Normal,
				Paladin,
				Joker
			};
			Type type;

			// 守护者·帕拉丁
			struct Paladin
			{
				const static Value defenseIncIndex = +100;
				const static Value damageDecIndex = -10;
				const static Value sunkInSilenceChanceIncIndex = +50;
				const static Value reboundDamageChanceDecIndex = -20;
				const static Value reboundDamageIncIndex = +100;
				const static Value agilityDecIndex = -20;
			};

			// 异面行者·小丑
			struct Joker
			{
				const static Value sunkInSilenceChanceIncIndex = +300;
				const static Value silentRoundsIncIndex = +1;
				const static Value reboundDamageChanceIncIndex = +50;
				const static Value defenseDecIndex = -10;
			};

			Career(Career::Type type);
		};

	public:
		Guardian(int level);
		Guardian(const Property& prop, Career::Type career);
		Guardian(const Guardian&) = default;
		Guardian& operator=(const Guardian&) = default;
		Guardian(Guardian&&) = default;
		Guardian& operator=(Guardian&&) = default;
		virtual ~Guardian();

	public:
		Career::Type GetCareer() const;
		String Attack(BasePlayer& opponent);
		Value  IsAttacked(Value damage);
		bool   SetPrimarySkill(Skill::Type primarySkill);
		bool   Promote(Career::Type career);

	private:
		Skill  m_skill;
		Career m_career;

	private:
		struct BasicProperties
		{
			static const Value	hitpoints = 800;
			static const Value	attack = 36;
			static const Value	defense = 70;
			static const Value	agility = 24;
			static const Value  armorRounds = 4;
		};

	private:
		void _LevelUpPropertiesDistributor_();
	};
	typedef Guardian * PGuardian;

	class Assassin : public BasePlayer
	{
	public:
		struct Skill : public SkillBase
		{
			enum class Type
			{
				TEARING, // 撕裂
				SLOW, // 减速
				INSPIRE // 精神鼓舞
			};
			Type  primarySkill;
			Value tearingChance;
			Value slowChance;

			Value slowIndex;

			static const Type mainSkill = Type::INSPIRE;
			Value fastenIndex;
			Value stolenIndex;

			Skill(Type primarySkill);
		};

		struct Career : public CareerBase
		{
			enum class Type
			{
				Normal,
				Yodian,
				Michelle
			};
			Type type;

			// 深渊猎手·尤迪安
			struct Yodian
			{
				const static Value tearingChanceIncIndex = +100;
				const static Value bleedRoundsIncIndex = +3;
				const static Value slowChanceIncIndex = +100;
				const static Value damageDecIndex = -20;
				const static Value intervalDecIndex = -200;
				const static Value stolenIncIndex = +50;
				const static Value hpointsDecIndex = -10;
				const static Value agilityIncIndex = +20;
			};

			// 审判者·米歇尔
			struct Michelle
			{
				const static Value slowChanceIncIndex = +400;
				const static Value slowRoundsIncIndex = +2;
				const static Value damageIncIndex = +100;
				const static Value defenseIncIndex = +30;
				const static Value stolenDecIndex = -20;
			};

			Career(Career::Type type);
		};

	public:
		Assassin(int level);
		Assassin(const Property& prop, Career::Type career);
		Assassin(const Assassin&) = default;
		Assassin& operator=(const Assassin&) = default;
		Assassin(Assassin&&) = default;
		Assassin& operator=(Assassin&&) = default;
		virtual ~Assassin();

	public:
		Career::Type GetCareer() const;
		String Attack(BasePlayer& opponent);
		Value  IsAttacked(Value damage);
		bool   SetPrimarySkill(Skill::Type primarySkill);
		bool   Promote(Career::Type career);

	private:
		Skill  m_skill;
		Career m_career;

	private:
		struct BasicProperties
		{
			static const Value hitpoints = 560;
			static const Value attack = 40;
			static const Value defense = 24;
			static const Value agility = 60;
			static const Value inspiredRounds = 3;
		};

	private:
		void _LevelUpPropertiesDistributor_();
	};
	typedef Assassin * PAssassin;
}