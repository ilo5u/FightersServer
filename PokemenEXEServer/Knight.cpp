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
		m_career(Career::Type::Normal),
		m_avatarRoundsCnt(0x0)
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
		m_career(career),
		m_avatarRoundsCnt(0x0)
	{
	}

	/// <summary>
	/// 
	/// </summary>
	Knight::~Knight()
	{
	}

	Knight::Skill::Skill(Type primarySkill)
	{
		this->primarySkill = primarySkill;
	}

	/// <summary>
	/// 
	/// </summary>
	String Knight::Attack(BasePlayer& opponent)
	{
		ZeroMemory(m_battleMessage, sizeof(m_battleMessage));
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
				sprintf(m_battleMessage + std::strlen(m_battleMessage),
					"武器恢复。");
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
				sprintf(m_battleMessage + std::strlen(m_battleMessage),
					"解除眩晕。");
			}
			else
			{
				--this->m_stateRoundsCnt.dizzying;
			}
			return false;
		}

		if (this->InState(State::DEAD))
		{
			return false;
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

			if (!this->InState(State::SILENT) && this->InState(State::ANGRIED))
			{	// 天神下凡
				this->m_anger = 0;
				this->SubState(State::ANGRIED);

				this->m_effects.avatar.attack
					= ConvertValueByPercent(this->m_property.m_attack, this->m_skill.avatarIndex);
				this->m_effects.avatar.defense
					= ConvertValueByPercent(this->m_property.m_defense, this->m_skill.avatarIndex);
				this->m_effects.avatar.agility
					= ConvertValueByPercent(this->m_property.m_agility, this->m_skill.avatarIndex);
				this->m_effects.avatar.interval
					= -this->m_skill.avatarIndex;

				this->m_property.m_attack += this->m_effects.avatar.attack;
				this->m_property.m_defense += this->m_effects.avatar.defense;
				this->m_property.m_agility += this->m_effects.avatar.agility;
				this->m_property.m_interval += this->m_effects.avatar.interval;

				this->m_avatarRoundsCnt = BasicProperties::avatarRounds;
				this->AddState(State::AVATAR);

				if (this->m_career.type == Career::Type::Ares)
				{
					this->m_property.m_attack
						+= ConvertValueByPercent(opponent.m_property.m_attack, Career::Ares::sunderArmIncIndex);
					this->m_property.m_defense
						= std::max<Value>(
							this->m_property.m_defense - 1,
							ConvertValueByPercent(opponent.m_property.m_defense, Career::Ares::sunderArmIncIndex)
							);
				}
				else if (this->m_career.type == Career::Type::Athena)
				{
					this->m_property.m_attack
						= std::min<Value>(
							this->m_property.m_attack - 1,
							opponent.m_property.m_attack
							);
					this->m_property.m_defense
						= std::max<Value>(
							this->m_property.m_defense + 1,
							ConvertValueByPercent(opponent.m_property.m_defense, Career::Athena::defenseIncIndex)
							);
				}
			}
			else if (!this->InState(State::SILENT))
			{
				switch (this->m_skill.primarySkill)
				{
				case Skill::Type::SUNDER_ARM:
				{
					if (_Hit_Target(this->m_skill.sunderArmChance, 0x0)
						&& !opponent.InState(State::SUNDERED))
					{
						opponent.m_effects.sundered.attack
							= ConvertValueByPercent(opponent.m_property.m_attack, this->m_skill.sunderArmIndex);
						opponent.m_property.m_attack
							+= opponent.m_effects.sundered.attack;
						opponent.AddState(State::SUNDERED);

						this->m_skill.sunderArmChance
							= std::min<Value>(40, this->m_skill.sunderArmChance + 1);
					}
					else if (_Hit_Target(this->m_skill.makeDizzyChance, 0x0)
						&& !opponent.InState(State::DIZZYING))
					{
						opponent.SetDizzyingRounds(CommonBasicValues::dizzyingRounds);
						opponent.AddState(State::DIZZYING);

						this->m_skill.makeDizzyChance
							= std::max<Value>(10, this->m_skill.makeDizzyChance - 2);
					}
				}
				break;

				case Skill::Type::MAKE_DIZZY:
				{
					if (_Hit_Target(this->m_skill.makeDizzyChance, 0x0))
					{
						opponent.SetDizzyingRounds(CommonBasicValues::dizzyingRounds);
						opponent.AddState(State::DIZZYING);

						this->m_skill.sunderArmChance
							= std::min<Value>(30, this->m_skill.sunderArmChance + 1);
					}
					else if (_Hit_Target(this->m_skill.sunderArmChance, 0x0))
					{
						opponent.m_effects.sundered.attack
							= ConvertValueByPercent(opponent.m_property.m_attack, this->m_skill.sunderArmIndex);
						opponent.m_property.m_attack
							+= opponent.m_effects.sundered.attack;
						opponent.AddState(State::SUNDERED);

						this->m_skill.makeDizzyChance
							= std::max<Value>(20, this->m_skill.makeDizzyChance - 1);
					}
				}
				break;

				default:
					break;
				}
			}

			Value damage = opponent.IsAttacked(AttackDamageCalculator(damage, opponent.GetDefense()));
			if (damage > 0 
				&& this->m_career.type != Career::Type::Ares)
			{	// 对方开启反甲
				sprintf(m_battleMessage + std::strlen(m_battleMessage),
					"受到%d点反伤。", damage);
				this->m_property.m_hpoints -= damage;
			}

			if (this->m_property.m_hpoints <= 0)
			{
				sprintf(m_battleMessage + std::strlen(m_battleMessage),
					"小精灵死亡。");
				this->m_property.m_hpoints = 0;
				this->m_state = State::DEAD;
			}
		}

		return { };
	}

	Value Knight::IsAttacked(Value damage)
	{
		return Value();
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