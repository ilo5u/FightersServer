#include "stdafx.h"

namespace Pokemen
{
	static const Strings namesOfGuardian{ "Caterpie", "Metapod", "Butterfree", "Weedle" };

	Guardian::Career::Career(Career::Type type) :
		type(type)
	{
	}

	/// <summary>
	/// 
	/// </summary>
	Guardian::Guardian(int level) :
		BasePlayer(
			PokemenType::GUARDIAN, namesOfGuardian[_Random(namesOfGuardian.size())],
			BasicProperties::hitpoints + _Random(CommonBasicValues::propertyOffset),
			BasicProperties::attack + _Random(CommonBasicValues::propertyOffset),
			BasicProperties::defense + _Random(CommonBasicValues::propertyOffset),
			BasicProperties::agility + _Random(CommonBasicValues::propertyOffset),
			0x0, 0x0, 0x0, 0x0
		),
		m_skill(Skill::Type::REBOUND_DAMAGE),
		m_career(Career::Type::Normal)
	{
		this->m_property.m_interval
			= IntervalValueCalculator(
				CommonBasicValues::interval,
				this->m_property.m_agility,
				this->m_property.m_attack
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

	Guardian::Guardian(const Property& prop, Career::Type career) :
		BasePlayer(prop),
		m_skill(Skill::Type::REBOUND_DAMAGE),
		m_career(career)
	{
	}

	/// <summary>
	/// 
	/// </summary>
	Guardian::~Guardian()
	{
	}

	Guardian::Skill::Skill(Type primarySkill)
	{
		this->primarySkill = primarySkill;
	}

	Guardian::Career::Type Guardian::GetCareer() const
	{
		return this->m_career.type;
	}

	String Guardian::Attack(BasePlayer& opponent)
	{
		return { };
	}

	Value Guardian::IsAttacked(Value damage)
	{
		return Value();
	}

	bool Guardian::SetPrimarySkill(Skill::Type primarySkill)
	{
		this->m_skill.primarySkill = primarySkill;
		return false;
	}

	bool Guardian::Promote(Career::Type career)
	{
		if (this->m_career.type == Career::Type::Normal)
		{
			this->m_career.type = career;
			switch (this->m_career.type)
			{
			case Career::Type::Paladin:
			{
				this->m_property.m_defense 
					+= ConvertValueByPercent(this->m_property.m_defense, Career::Paladin::defenseIncIndex);
				this->m_property.m_attack
					+= ConvertValueByPercent(this->m_property.m_attack, Career::Paladin::damageDecIndex);
				this->m_skill.sunkInSilenceChance
					+= (this->m_skill.sunkInSilenceChance, Career::Paladin::sunkInSilenceChanceIncIndex);
				this->m_skill.reboundDamageChance
					+= (this->m_skill.reboundDamageChance, Career::Paladin::reboundDamageChanceDecIndex);
				this->m_skill.reboundDamageIndex 
					+= (this->m_skill.reboundDamageIndex, Career::Paladin::reboundDamageIncIndex);
				this->m_property.m_agility 
					+= (this->m_property.m_agility, Career::Paladin::agilityDecIndex);
			}
			break;

			case Career::Type::Joker:
			{
				this->m_skill.sunkInSilenceChance
					+= (this->m_skill.sunkInSilenceChance, Career::Joker::sunkInSilenceChanceIncIndex);
				this->m_skill.reboundDamageChance
					+= (this->m_skill.reboundDamageChance, Career::Joker::reboundDamageChanceIncIndex);
				this->m_property.m_defense 
					+= (this->m_property.m_defense, Career::Joker::defenseDecIndex);
			}
			break;

			default:
				break;
			}
		}
		return false;
	}

	void Guardian::_LevelUpPropertiesDistributor_()
	{
		Value attackInc = 0;
		Value defenseInc = 0;
		Value agilityInc = 0;

		switch (this->m_career.type)
		{
		case Career::Type::Normal:
		{
			defenseInc 
				= _Random(CommonBasicValues::levelupPropertiesInc / 2);
			attackInc 
				= _Random(CommonBasicValues::levelupPropertiesInc / 2);
			agilityInc 
				= CommonBasicValues::levelupPropertiesInc - defenseInc - attackInc;

			this->m_property.m_hpoints
				+= _Random(static_cast<Value>((double)this->m_property.m_hpoints / 15.0)) + CommonBasicValues::hpointsInc;
		}
		break;

		case Career::Type::Paladin:
		{
			defenseInc 
				= _Random(static_cast<Value>(CommonBasicValues::levelupPropertiesInc / 2), CommonBasicValues::levelupPropertiesInc);
			attackInc 
				= _Random(CommonBasicValues::levelupPropertiesInc - defenseInc);
			agilityInc 
				= CommonBasicValues::levelupPropertiesInc - defenseInc - agilityInc;
			defenseInc 
				+= ConvertValueByPercent(defenseInc, Career::Paladin::defenseIncIndex);

			this->m_property.m_hpoints += _Random(static_cast<Value>((double)this->m_property.m_hpoints / 10.0)) + CommonBasicValues::hpointsInc;
		}
		break;

		case Career::Type::Joker:
		{
			agilityInc = _Random(static_cast<Value>(CommonBasicValues::levelupPropertiesInc / 2), CommonBasicValues::levelupPropertiesInc);
			attackInc = _Random(CommonBasicValues::levelupPropertiesInc - agilityInc);
			defenseInc = CommonBasicValues::levelupPropertiesInc - agilityInc - attackInc;
			defenseInc -= ConvertValueByPercent(defenseInc, Career::Joker::defenseDecIndex);

			this->m_property.m_hpoints += _Random(static_cast<Value>((double)this->m_property.m_hpoints / 20.0)) + CommonBasicValues::hpointsInc;
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
