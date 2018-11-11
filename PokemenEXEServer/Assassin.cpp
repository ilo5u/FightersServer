#include "stdafx.h"

namespace Pokemen
{
	static const Strings namesOfAssassin{ "Kakuna", "Beedrill", "Pidgey", "Pidgeotto" };

	Assassin::Career::Career(Career::Type type) :
		type(type)
	{
	}

	/// <summary>
	/// 
	/// </summary>
	Assassin::Assassin(int level) :
		BasePlayer(
			PokemenType::ASSASSIN, namesOfAssassin[_Random(namesOfAssassin.size())],
			BasicProperties::hitpoints + _Random(CommonBasicValues::propertyOffset),
			BasicProperties::attack + _Random(CommonBasicValues::propertyOffset),
			BasicProperties::defense + _Random(CommonBasicValues::propertyOffset),
			BasicProperties::agility + _Random(CommonBasicValues::propertyOffset),
			0x0, 0x0, 0x0, 0x0
		),
		m_skill(Skill::Type::TEARING),
		m_career(Career::Type::Normal)
	{
		this->m_property.m_interval
			= IntervalValueCalculator(
				CommonBasicValues::interval,
				this->m_property.m_agility,
				this->m_property.m_defense
			);
		this->m_property.m_critical
			= CriticalValueCalculator(
				CommonBasicValues::critical,
				this->m_property.m_agility
			);
		this->m_property.m_hitratio
			= HitratioValueCalculator(
				CommonBasicValues::hitratio,
				this->m_property.m_agility,
				this->m_property.m_attack
			);
		this->m_property.m_parryratio
			= ParryratioValueCalculator(
				CommonBasicValues::parryratio,
				this->m_property.m_agility,
				this->m_property.m_defense
			);

		while (this->m_property.m_level < std::min<Value>(level, CommonBasicValues::levelLimitation))
			Upgrade(CommonBasicValues::exp);
	}

	Assassin::Assassin(const Property& prop, Career::Type career) :
		BasePlayer(prop),
		m_skill(Skill::Type::TEARING),
		m_career(career)
	{
	}

	/// <summary>
	/// 
	/// </summary>
	Assassin::~Assassin()
	{
	}

	Assassin::Skill::Skill(Type primarySkill)
	{
		this->primarySkill = primarySkill;
	}

	String Assassin::Attack(BasePlayer& opponent)
	{
		return {};
	}

	Value Assassin::IsAttacked(Value damage)
	{
		return Value();
	}

	bool Assassin::SetPrimarySkill(Skill::Type primarySkill)
	{
		this->m_skill.primarySkill = primarySkill;
		return false;
	}

	bool Assassin::Promote(Career::Type career)
	{
		if (this->m_career.m_type == Career::Type::Normal)
		{
			this->m_career.m_type = career;
			switch (this->m_career.m_type)
			{
			case Career::Type::Yodian:
			{
				this->m_skill.tearingChance
					+= ConvertValueByPercent(this->m_skill.tearingChance, Career::Yodian::tearingChanceIncIndex);
				this->m_skill.slowChance 
					+= ConvertValueByPercent(this->m_skill.slowChance, Career::Yodian::slowChanceIncIndex);
				this->m_property.m_attack
					+= ConvertValueByPercent(this->m_property.m_attack, Career::Yodian::damageDecIndex);
				this->m_property.m_interval 
					+= Career::Yodian::intervalDecIndex;
				this->m_skill.stolenIndex 
					+= ConvertValueByPercent(this->m_skill.stolenIndex, Career::Yodian::stolenIncIndex);
				this->m_property.m_hpoints 
					+= ConvertValueByPercent(this->m_property.m_hpoints, Career::Yodian::hpointsDecIndex);
				this->m_property.m_agility 
					+= ConvertValueByPercent(this->m_property.m_agility, Career::Yodian::agilityIncIndex);
			}
			break;

			case Career::Type::Michelle:
			{
				this->m_skill.slowChance 
					+= ConvertValueByPercent(this->m_skill.slowChance, Career::Michelle::slowChanceIncIndex);
				this->m_property.m_attack 
					+= ConvertValueByPercent(this->m_property.m_attack, Career::Michelle::damageIncIndex);
				this->m_property.m_defense
					+= ConvertValueByPercent(this->m_property.m_defense, Career::Michelle::defenseIncIndex);
				this->m_skill.stolenIndex
					+= ConvertValueByPercent(this->m_skill.stolenIndex, Career::Michelle::stolenDecIndex);
			}
			break;

			default:
				break;
			}
		}
		return false;
	}

	const static double PI = 3.1415926;
	void Assassin::_LevelUpPropertiesDistributor_()
	{
		Value attackInc = 0;
		Value defenseInc = 0;
		Value agilityInc = 0;

		switch (this->m_career.m_type)
		{
		case Career::Type::Normal:
		{
			attackInc
				= _Random(CommonBasicValues::levelupPropertiesInc / 4);
			defenseInc 
				= _Random(CommonBasicValues::levelupPropertiesInc / 4);
			agilityInc
				= CommonBasicValues::levelupPropertiesInc - attackInc - defenseInc;

			this->m_property.m_hpoints 
				+= _Random(static_cast<Value>((double)this->m_property.m_hpoints / 15.0)) + CommonBasicValues::hpointsInc;
		}
		break;

		case Career::Type::Yodian:
		{
			attackInc 
				= _Random(static_cast<Value>(CommonBasicValues::levelupPropertiesInc / 3), CommonBasicValues::levelupPropertiesInc);
			agilityInc 
				= _Random(CommonBasicValues::levelupPropertiesInc - attackInc);
			defenseInc 
				= CommonBasicValues::levelupPropertiesInc - attackInc - agilityInc;
			attackInc 
				+= ConvertValueByPercent(attackInc, Career::Yodian::damageDecIndex);	// special
			agilityInc 
				+= ConvertValueByPercent(agilityInc, Career::Yodian::agilityIncIndex);

			this->m_property.m_hpoints 
				+= CommonBasicValues::hpointsInc - _Random(static_cast<Value>((double)this->m_property.m_hpoints / 20.0));
		}
		break;

		case Career::Type::Michelle:
		{
			attackInc = _Random(static_cast<Value>(CommonBasicValues::levelupPropertiesInc / 3), static_cast<Value>(CommonBasicValues::levelupPropertiesInc / 2));
			defenseInc = _Random(CommonBasicValues::levelupPropertiesInc - attackInc);
			agilityInc = CommonBasicValues::levelupPropertiesInc - attackInc - defenseInc;
			attackInc += ConvertValueByPercent(attackInc, Career::Michelle::damageIncIndex);
			defenseInc += ConvertValueByPercent(defenseInc, Career::Michelle::defenseIncIndex);

			this->m_property.m_hpoints += static_cast<Value>(std::sqrt(std::sqrt(_Random(static_cast<Value>(CommonBasicValues::hpointsInc / 2), CommonBasicValues::hpointsInc)) * CommonBasicValues::hpointsInc) * PI);
		}
		break;

		default:
			break;
		}
		this->m_property.m_attack += attackInc;
		this->m_property.m_defense += defenseInc;
		this->m_property.m_agility += agilityInc;
	}
}
