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

	protected:
		struct SkillBase { };
		struct CareerBase { };

	protected:
		static Value IntervalValueCalculator(Value base, Value primaryAffect, Value secondaryAffect);
		static Value CriticalValueCalculator(Value base, Value affect);
		static Value HitratioValueCalculator(Value base, Value primaryAffect, Value secondaryAffect);
		static Value ParryratioValueCalculator(Value base, Value primaryAffect, Value secondaryAffect);
		static Value ConvertValueByPercent(Value base, Value index);

	protected:
		struct CommonBasicValues
		{
			static const Value propertyOffset = 4;

			static const Value interval = 1000;
			static const Value critical = 20;
			static const Value hitratio = 90;
			static const Value parryratio = 10;

			static const Value hpointsInc = 100;
		};

	protected:
		Property m_property;
	};
	typedef BasePlayer * PBasePlayer;

	class Master : public BasePlayer
	{
	public:
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
				const static Value damageDecIndex = -10;
				const static Value hpointsIncIndex = +20;
			};

			// 黑暗大法师
			struct Darker
			{
				const static Value defenseDecIndex = -10;
				const static Value damageIncIndex = +30;
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
		bool   Promote(Career::Type career);

	private:
		Career m_career;

	private:
		struct BasicProperties
		{
			static const Value hitpoints = 960;
			static const Value attack = 36;
			static const Value defense = 40;
			static const Value agility = 24;
		};
	};
	typedef Master * PMaster;

	class Knight : public BasePlayer
	{
	public:
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
				const static Value damageIncIndex = +25;
				const static Value intervalIncIndex = +200;
			};

			// 战争女神·雅典娜
			struct Athena
			{
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
		bool   Promote(Career::Type career);

	private:
		Career m_career;

	private:
		struct BasicProperties
		{
			static const Value	hitpoints = 640;
			static const Value	attack = 50;
			static const Value	defense = 36;
			static const Value	agility = 36;
		};
	};
	typedef Knight * PKnight;

	class Guardian : public BasePlayer
	{
	public:
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
				const static Value agilityDecIndex = -20;
			};

			// 异面行者·小丑
			struct Joker
			{
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
		bool   Promote(Career::Type career);

	private:
		Career m_career;

	private:
		struct BasicProperties
		{
			static const Value	hitpoints = 800;
			static const Value	attack = 36;
			static const Value	defense = 70;
			static const Value	agility = 24;
		};
	};
	typedef Guardian * PGuardian;

	class Assassin : public BasePlayer
	{
	public:
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
				const static Value damageDecIndex = -20;
				const static Value intervalDecIndex = -200;
				const static Value hpointsDecIndex = -10;
				const static Value agilityIncIndex = +20;
			};

			// 审判者·米歇尔
			struct Michelle
			{
				const static Value damageIncIndex = +100;
				const static Value defenseIncIndex = +30;
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
		bool   Promote(Career::Type career);

	private:
		Career m_career;

	private:
		struct BasicProperties
		{
			static const Value hitpoints = 560;
			static const Value attack = 40;
			static const Value defense = 24;
			static const Value agility = 60;
		};
	};
	typedef Assassin * PAssassin;
}