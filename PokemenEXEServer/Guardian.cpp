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

	Guardian::Skill::Skill(Type primarySkill) :
		primarySkill(primarySkill),
		sunkInSilenceChance(+20), reboundDamageChance(+40),
		reboundDamageIndex(+50), defenseIndex(+200)
	{
	}

	Guardian::Career::Type Guardian::GetCareer() const
	{
		return this->m_career.type;
	}

	String Guardian::Attack(BasePlayer& opponent)
	{
		this->m_battleMessage[0] = 0x0;

		/* ×´Ì¬ÅÐ¾ö */
		if (this->InState(State::DEAD))
			return { };

		if (this->InState(State::ARMOR))
		{
			if (this->m_stateRoundsCnt.armor == 1)
			{
				this->m_property.m_defense -=
					this->m_effects.armor.defense;
				this->SubState(State::ARMOR);
			}
			else
			{
				--this->m_stateRoundsCnt.armor;
			}
		}

		if (this->InState(State::SILENT))
		{
			if (this->m_stateRoundsCnt.silent == 1)
			{
				this->SubState(State::SILENT);
			}
			else
			{
				--this->m_stateRoundsCnt.silent;
			}
		}

		if (this->InState(State::SLOWED))
		{
			if (this->m_stateRoundsCnt.slowed == 1)
			{
				this->m_property.m_interval -= this->m_effects.slowed.interval;
				this->SubState(State::SLOWED);
			}
			else
			{
				--this->m_stateRoundsCnt.slowed;
			}
		}

		if (this->InState(State::SUNDERED))
		{
			if (this->m_stateRoundsCnt.sundered == 1)
			{
				this->m_property.m_attack -= this->m_effects.sundered.attack;
				this->SubState(State::SUNDERED);
			}
			else
			{
				--this->m_stateRoundsCnt.sundered;
			}
		}

		if (this->InState(State::DIZZYING))
		{
			if (this->m_stateRoundsCnt.dizzying == 1)
			{
				this->SubState(State::DIZZYING);
				return { };
			}
			else
			{
				--this->m_stateRoundsCnt.dizzying;
			}
		}

		/* ¹¥»÷ÅÐ¾ö */
		if (_Hit_Target(this->m_property.m_hitratio, opponent.GetParryratio()))
		{
			Value damage = this->m_property.m_attack;

			if (_Hit_Target(this->m_property.m_critical, opponent.GetCritical()))
			{ // ±©»÷
				damage = static_cast<Value>((double)damage * 1.5);
			}
			
			/* ¼¼ÄÜÅÐ¾ö */
			if (!this->InState(State::SILENT) && this->InState(State::ANGRIED))
			{
				sprintf(this->m_battleMessage + std::strlen(this->m_battleMessage),
					"È«¸±Îä×°¡£");
				this->m_anger = 0;
				this->SubState(State::ANGRIED);
				/* È«¸±Îä×° */
				this->m_effects.armor.defense
					+= ConvertValueByPercent(this->m_property.m_defense, this->m_skill.defenseIndex);
				this->m_stateRoundsCnt.armor = BasicProperties::armorRounds;
				switch (this->m_career.type)
				{
				case Career::Type::Paladin:
					this->m_effects.armor.defense
						+= ConvertValueByPercent(this->m_effects.armor.defense, Career::Paladin::defenseIncIndex);
					++this->m_stateRoundsCnt.armor;
					break;

				case Career::Type::Joker:
					this->m_effects.armor.defense
						+= ConvertValueByPercent(this->m_effects.armor.defense, Career::Joker::defenseDecIndex);
					--this->m_stateRoundsCnt.armor;
					break;

				default:
					break;
				}
				this->m_property.m_defense += this->m_effects.armor.defense;
				this->AddState(State::ARMOR);
			}
			else if (!this->InState(State::SILENT))
			{
				switch (this->m_skill.primarySkill)
				{
				case Skill::Type::REBOUND_DAMAGE:
					/* Ö÷ÐÞ±³´Ì */
				{
					if (_Hit_Target(this->m_skill.reboundDamageChance, 0))
					{
						/* ±³´Ì */
						sprintf(this->m_battleMessage + std::strlen(this->m_battleMessage),
							"±³´Ì¡£");
						this->AddState(State::REBOUND);
					}
					else if (_Hit_Target(this->m_skill.sunkInSilenceChance, 5))
					{
						/* ³ÁÄ¬ */
						sprintf(this->m_battleMessage + std::strlen(this->m_battleMessage),
							"³ÁÄ¬¡£");
						opponent.SetSilentRounds(CommonBasicValues::silentRounds);
						opponent.AddState(State::SILENT);
					}
				}
				break;

				case Skill::Type::SUNK_IN_SILENCE:
				{
					/* Ö÷ÐÞ³ÁÄ¬ */
					if (_Hit_Target(this->m_skill.sunkInSilenceChance, 0))
					{
						/* ³ÁÄ¬ */
						sprintf(this->m_battleMessage + std::strlen(this->m_battleMessage),
							"³ÁÄ¬¡£");
						opponent.SetSilentRounds(CommonBasicValues::silentRounds);
						opponent.AddState(State::SILENT);
					}
					else if (_Hit_Target(this->m_skill.reboundDamageChance, 5))
					{
						/* ±³´Ì */
						sprintf(this->m_battleMessage + std::strlen(this->m_battleMessage),
							"±³´Ì¡£");
						this->AddState(State::REBOUND);
					}
				}
				break;

				default:
					break;
				}
			}

			// ¹¥»÷µÐ·½Ð¡¾«Áé
			/* ÉËº¦ÅÐ¾ö */
			sprintf(m_battleMessage + std::strlen(m_battleMessage),
				"Ôì³É%dµãÉËº¦¡£",
				AttackDamageCalculator(damage, opponent.GetDefense()));
			Value rebounce = opponent.IsAttacked(AttackDamageCalculator(damage, opponent.GetDefense()));
			if (rebounce > 0)
			{	// ¶Ô·½¿ªÆô·´¼×
				sprintf(m_battleMessage + std::strlen(m_battleMessage),
					"ÊÜµ½%dµã·´ÉË¡£", rebounce);
				this->m_property.m_hpoints -= rebounce;
			}

			if (this->m_property.m_hpoints <= 0)
			{
				sprintf(m_battleMessage + std::strlen(m_battleMessage),
					"Ð¡¾«ÁéËÀÍö¡£");
				this->m_property.m_hpoints = 0;
				this->m_state = State::DEAD;
			}
		}
		else
		{
			sprintf(m_battleMessage + std::strlen(m_battleMessage), "Î´ÃüÖÐ¡£");
		}

		return m_battleMessage;
	}

	Value Guardian::IsAttacked(Value damage)
	{
		Value back = 0;
		if (damage >= this->m_property.m_hpoints)
		{
			this->m_property.m_hpoints = 0;
			this->m_state = State::DEAD;
		}
		else
		{
			this->m_property.m_hpoints -= damage;
			this->m_anger = std::min<Value>(
				CommonBasicValues::angerLimitation,
				this->m_anger + CommonBasicValues::angerInc + _Random(CommonBasicValues::angerInc)
				);

			if (this->m_anger == CommonBasicValues::angerLimitation)
				this->AddState(State::ANGRIED);

			/* ³öÑª */
			if (this->InState(State::BLEED))
			{
				this->m_property.m_hpoints -= BloodingDamageCalculator(CommonBasicValues::bleedDamage, this->m_property.m_defense);
				sprintf(this->m_battleMessage + std::strlen(this->m_battleMessage),
					"³öÑªÊÜµ½%dµãÉËº¦¡£",
					BloodingDamageCalculator(CommonBasicValues::bleedDamage, this->m_property.m_defense));
				if (this->m_property.m_hpoints <= 0)
				{
					this->m_property.m_hpoints = 0;
					this->m_state = State::DEAD;
				}

				if (this->m_stateRoundsCnt.bleed == 1)
					this->SubState(State::BLEED);
			}

			/* ±³´Ì */
			if (this->InState(State::REBOUND))
			{
				back = ConvertValueByPercent(damage, this->m_skill.reboundDamageIndex);
				switch (this->m_career.type)
				{
				case Career::Type::Paladin:
					back += ConvertValueByPercent(back, Career::Paladin::reboundDamageIncIndex);
					break;

				case Career::Type::Joker:
					break;

				default:
					break;
				}
				this->SubState(State::REBOUND);
			}
		}
		return back;
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
