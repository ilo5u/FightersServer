#include "stdafx.h"

namespace Pokemen
{
	static const Strings namesOfKnight{ "Charizard", "Squirtle", "Wartortle", "Blastoise" };

	Knight::Career::Career(Career::Type type) :
		type(type)
	{
	}

	/// <summary>
	/// 
	/// </summary>
	Knight::Knight(int level) :
		BasePlayer(
			PokemenType::KNIGHT, namesOfKnight[_Random(namesOfKnight.size())],
			BasicProperties::hitpoints + _Random(CommonBasicValues::propertyOffset),
			BasicProperties::attack + _Random(CommonBasicValues::propertyOffset),
			BasicProperties::defense + _Random(CommonBasicValues::propertyOffset),
			BasicProperties::agility + _Random(CommonBasicValues::propertyOffset),
			0x0, 0x0, 0x0, 0x0
		),
		m_skill(Skill::Type::SUNDER_ARM),
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
				this->m_property.m_defense
			);
		this->m_property.m_parryratio
			= ParryratioValueCalculator(
				CommonBasicValues::parryratio,
				this->m_property.m_agility,
				this->m_property.m_attack
			);

		while (this->m_property.m_level < std::min<Value>(level, CommonBasicValues::levelLimitation))
			Upgrade(CommonBasicValues::exp);
	}

	Knight::Knight(const Property& prop, Career::Type career) :
		BasePlayer(prop),
		m_skill(Skill::Type::SUNDER_ARM),
		m_career(career)
	{
	}

	/// <summary>
	/// 
	/// </summary>
	Knight::~Knight()
	{
	}

	Knight::Skill::Skill(Type primarySkill) :
		primarySkill(primarySkill),
		sunderArmChance(+10), makeDizzyChance(+5),
		sunderArmIndex(-10), avatarIndex(+100)
	{
	}

	Knight::Career::Type Knight::GetCareer() const
	{
		return this->m_career.type;
	}

	/// <summary>
	/// 
	/// </summary>
	String Knight::Attack(BasePlayer& opponent)
	{
		m_battleMessage[0] = 0x0;
		// 处理异常状态
		if (this->InState(State::AVATAR))
		{
			if (this->m_stateRoundsCnt.avatar == 1)
			{
				this->m_property.m_attack
					-= this->m_effects.avatar.attack;
				this->m_property.m_defense 
					-= this->m_effects.avatar.defense;
				this->m_property.m_agility 
					-= this->m_effects.avatar.agility;
				this->m_property.m_interval
					-= this->m_effects.avatar.interval;
				this->SubState(State::AVATAR);
			}
			else
			{
				--this->m_stateRoundsCnt.avatar;
			}
		}

		if (this->InState(State::SUNDERED))
		{
			if (this->m_stateRoundsCnt.sundered == 1)
			{
				this->m_property.m_attack 
					-= this->m_effects.sundered.attack;
				this->SubState(State::SUNDERED);
			}
			else
			{
				--this->m_stateRoundsCnt.sundered;
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

		if (this->InState(State::DIZZYING))
		{
			if (this->m_stateRoundsCnt.dizzying == 1)
			{
				this->SubState(State::DIZZYING);
			}
			else
			{
				--this->m_stateRoundsCnt.dizzying;
			}
			return m_battleMessage;
		}

		if (this->InState(State::DEAD))
		{
			return m_battleMessage;
		}

		if (_Hit_Target(this->m_property.m_hitratio, opponent.GetParryratio()))
		{
			Value damage = this->m_property.m_attack;
			if (_Hit_Target(this->m_property.m_critical, 0x0))
			{	// 暴击
				switch (this->m_career.type)
				{
				case Career::Type::Normal:
					damage = static_cast<Value>((double)damage * 1.5);
					break;

				case Career::Type::Ares:
					damage = static_cast<Value>((double)damage * 2.0);
					break;

				case Career::Type::Athena:
					damage = static_cast<Value>((double)damage * 1.8);
					break;

				default:
					break;
				}
			}

			if (this->InState(State::ANGRIED))
			{	// 天神下凡
				sprintf(m_battleMessage + std::strlen(m_battleMessage),
					"天神下凡。");
				this->m_anger = 0;
				this->SubState(State::ANGRIED);

				if (this->m_skill.avatarIndex != +100)
					this->m_skill.avatarIndex = +100;

				this->m_effects.avatar.attack
					= ConvertValueByPercent(this->m_property.m_attack, this->m_skill.avatarIndex);
				this->m_effects.avatar.defense
					= ConvertValueByPercent(this->m_property.m_defense, this->m_skill.avatarIndex);
				this->m_effects.avatar.agility
					= ConvertValueByPercent(this->m_property.m_agility, this->m_skill.avatarIndex);
				this->m_effects.avatar.interval
					= -this->m_skill.avatarIndex;

				if (this->m_career.type == Career::Type::Ares)
				{
					this->m_effects.avatar.attack
						+= ConvertValueByPercent(opponent.m_property.m_attack, Career::Ares::sunderArmIncIndex);
					this->m_effects.avatar.defense
						+= std::max<Value>(
							this->m_property.m_defense - 1,
							ConvertValueByPercent(opponent.m_property.m_defense, Career::Ares::sunderArmIncIndex)
							);
				}
				else if (this->m_career.type == Career::Type::Athena)
				{
					this->m_effects.avatar.attack
						+= std::min<Value>(
							this->m_property.m_attack - 1,
							opponent.m_property.m_attack
							);
					this->m_effects.avatar.defense
						+= std::max<Value>(
							this->m_property.m_defense + 1,
							ConvertValueByPercent(opponent.m_property.m_defense, Career::Athena::defenseIncIndex)
							);
				}

				this->m_property.m_attack += this->m_effects.avatar.attack;
				this->m_property.m_defense += this->m_effects.avatar.defense;
				this->m_property.m_agility += this->m_effects.avatar.agility;
				this->m_property.m_interval += this->m_effects.avatar.interval;

				this->m_stateRoundsCnt.avatar = BasicProperties::avatarRounds;
				this->AddState(State::AVATAR);
			}
			else if (!this->InState(State::SILENT))
			{
				switch (this->m_skill.primarySkill)
				{
				case Skill::Type::SUNDER_ARM:
				{
					if (_Hit_Target(this->m_skill.sunderArmChance, 0)
						&& !opponent.InState(State::SUNDERED))
					{
						opponent.m_effects.sundered.attack
							= ConvertValueByPercent(opponent.m_property.m_attack, this->m_skill.sunderArmIndex);
						opponent.m_property.m_attack
							+= opponent.m_effects.sundered.attack;
						opponent.SetSunderedRounds(CommonBasicValues::sunderedRounds);
						opponent.AddState(State::SUNDERED);
						sprintf(m_battleMessage + std::strlen(m_battleMessage),
							"致残。");

						this->m_skill.sunderArmChance
							= std::min<Value>(40, this->m_skill.sunderArmChance + 1);
					}
					else if (_Hit_Target(this->m_skill.makeDizzyChance, 5)
						&& !opponent.InState(State::DIZZYING))
					{
						opponent.SetDizzyingRounds(CommonBasicValues::dizzyingRounds);
						opponent.AddState(State::DIZZYING);
						sprintf(m_battleMessage + std::strlen(m_battleMessage),
							"践踏。");

						this->m_skill.makeDizzyChance
							= std::max<Value>(10, this->m_skill.makeDizzyChance - 2);
					}
				}
				break;

				case Skill::Type::MAKE_DIZZY:
				{
					if (_Hit_Target(this->m_skill.makeDizzyChance, 0))
					{
						opponent.SetDizzyingRounds(CommonBasicValues::dizzyingRounds);
						opponent.AddState(State::DIZZYING);
						sprintf(m_battleMessage + std::strlen(m_battleMessage),
							"践踏。");

						this->m_skill.sunderArmChance
							= std::min<Value>(30, this->m_skill.sunderArmChance + 1);
					}
					else if (_Hit_Target(this->m_skill.sunderArmChance, 5))
					{
						opponent.m_effects.sundered.attack
							= ConvertValueByPercent(opponent.m_property.m_attack, this->m_skill.sunderArmIndex);
						opponent.m_property.m_attack
							+= opponent.m_effects.sundered.attack;
						opponent.SetSunderedRounds(CommonBasicValues::sunderedRounds);
						opponent.AddState(State::SUNDERED);
						sprintf(m_battleMessage + std::strlen(m_battleMessage),
							"致残。");

						this->m_skill.makeDizzyChance
							= std::max<Value>(20, this->m_skill.makeDizzyChance - 1);
					}
				}
				break;

				default:
					break;
				}
			}

			sprintf(m_battleMessage + std::strlen(m_battleMessage),
				"造成%d点伤害。",
				AttackDamageCalculator(damage, opponent.GetDefense()));
			Value rebounce = opponent.IsAttacked(AttackDamageCalculator(damage, opponent.GetDefense()));
			if (rebounce > 0 
				&& this->m_career.type != Career::Type::Ares)
			{	// 对方开启反甲
				sprintf(m_battleMessage + std::strlen(m_battleMessage),
					"受到%d点反伤。", rebounce);
				this->m_property.m_hpoints -= rebounce;
			}

			if (this->m_property.m_hpoints <= 0)
			{
				sprintf(m_battleMessage + std::strlen(m_battleMessage),
					"小精灵死亡。");
				this->m_property.m_hpoints = 0;
				this->m_state = State::DEAD;
			}
		}
		else
		{
			sprintf(m_battleMessage + std::strlen(m_battleMessage), "未命中。");
		}

		return m_battleMessage;
	}

	Value Knight::IsAttacked(Value damage)
	{
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

			if (this->InState(State::BLEED))
			{
				this->m_property.m_hpoints -=
					BloodingDamageCalculator(CommonBasicValues::bleedDamage, this->m_property.m_defense);
				sprintf(this->m_battleMessage + std::strlen(this->m_battleMessage),
					"出血受到%d点伤害。",
					BloodingDamageCalculator(CommonBasicValues::bleedDamage, this->m_property.m_defense));
				if (this->m_property.m_hpoints <= 0)
				{
					this->m_property.m_hpoints = 0;
					this->m_state = State::DEAD;
				}

				if (this->m_stateRoundsCnt.bleed == 1)
					this->SubState(State::BLEED);
			}
		}
		return 0;
	}

	bool Knight::SetPrimarySkill(Skill::Type primarySkill)
	{
		this->m_skill.primarySkill = primarySkill;
		return false;
	}

	bool Knight::Promote(Career::Type career)
	{
		if (this->m_career.type == Career::Type::Normal)
		{
			this->m_career.type = career;
			switch (this->m_career.type)
			{
			case Career::Type::Ares:
			{
				this->m_skill.sunderArmChance
					+= ConvertValueByPercent(this->m_skill.sunderArmChance, Career::Ares::sunderArmChanceIncIndex);
				this->m_skill.sunderArmIndex 
					+= ConvertValueByPercent(this->m_skill.sunderArmIndex, Career::Ares::sunderArmIncIndex);
				this->m_property.m_attack 
					+= ConvertValueByPercent(this->m_property.m_attack, Career::Ares::damageIncIndex);
				this->m_property.m_interval
					+= Career::Ares::intervalIncIndex;
			}
			break;

			case Career::Type::Athena:
			{
				this->m_skill.makeDizzyChance
					+= ConvertValueByPercent(this->m_skill.makeDizzyChance, Career::Athena::makeDizzyChanceIncIndex);
				this->m_property.m_defense
					+= ConvertValueByPercent(this->m_property.m_defense, Career::Athena::defenseIncIndex);
				this->m_property.m_attack 
					+= ConvertValueByPercent(this->m_property.m_attack, Career::Athena::damageDecIndex);
				this->m_property.m_interval
					+= Career::Athena::intervalDecIndex;
			}
			break;

			default:
				break;
			}
		}
		return false;
	}

	void Knight::_LevelUpPropertiesDistributor_()
	{
		Value attackInc = 0;
		Value defenseInc = 0;
		Value agilityInc = 0;

		switch (this->m_career.type)
		{
		case Career::Type::Normal:
		{
			attackInc 
				= _Random(CommonBasicValues::levelupPropertiesInc / 2);
			defenseInc 
				= _Random(CommonBasicValues::levelupPropertiesInc / 2);
			agilityInc 
				= CommonBasicValues::levelupPropertiesInc - attackInc - defenseInc;

			this->m_property.m_hpoints
				+= _Random(static_cast<Value>((double)this->m_property.m_hpoints / 10.0)) + CommonBasicValues::hpointsInc;
		}
		break;

		case Career::Type::Ares:
		{
			attackInc
				= _Random(static_cast<Value>(CommonBasicValues::levelupPropertiesInc / 2), CommonBasicValues::levelupPropertiesInc);
			agilityInc 
				= _Random(CommonBasicValues::levelupPropertiesInc - attackInc);
			defenseInc
				= CommonBasicValues::levelupPropertiesInc - attackInc - agilityInc;
			attackInc 
				+= ConvertValueByPercent(attackInc, Career::Ares::damageIncIndex);

			this->m_property.m_hpoints 
				+= _Random(static_cast<Value>((double)this->m_property.m_hpoints / 10.0)) + CommonBasicValues::hpointsInc;
		}
		break;

		case Career::Type::Athena:
		{
			attackInc
				= _Random(static_cast<Value>(CommonBasicValues::levelupPropertiesInc / 2), CommonBasicValues::levelupPropertiesInc);
			attackInc 
				+= ConvertValueByPercent(attackInc, Career::Athena::damageDecIndex);
			agilityInc 
				= _Random(CommonBasicValues::levelupPropertiesInc - attackInc);
			defenseInc 
				= CommonBasicValues::levelupPropertiesInc - attackInc - agilityInc;

			this->m_property.m_hpoints
				+= _Random(static_cast<Value>((double)this->m_property.m_hpoints / 15.0)) + CommonBasicValues::hpointsInc;
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