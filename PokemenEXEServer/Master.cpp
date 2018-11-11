#include "stdafx.h"

namespace Pokemen
{
	static const Strings namesOfMaster{ "Bulbasaur", "Ivysaur", "Venusaur", "Charmander" };

	Master::Career::Career(Career::Type type) :
		type(type)
	{
	}

	Master::Master(int level) :
		BasePlayer(
			PokemenType::MASTER, namesOfMaster[_Random(namesOfMaster.size())],
			BasicProperties::hitpoints + _Random(CommonBasicValues::propertyOffset),
			BasicProperties::attack + _Random(CommonBasicValues::propertyOffset),
			BasicProperties::defense + _Random(CommonBasicValues::propertyOffset),
			BasicProperties::agility + _Random(CommonBasicValues::propertyOffset),
			0x0, 0x0, 0x0, 0x0
		),
		m_skill(Skill::Type::RAGED),
		m_career(Career::Type::Normal),
		m_angriedCnt(0x0)
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

	Master::Master(const Property& prop, Career::Type career) :
		BasePlayer(prop),
		m_skill(Skill::Type::RAGED),
		m_career(career),
		m_angriedCnt(0x0)
	{
	}

	Master::~Master()
	{
	}

	Master::Skill::Skill(Type primarySkill)
	{
		this->primarySkill = primarySkill;
	}

	Master::Career::Type Master::GetCareer() const
	{
		return this->m_career.type;
	}

	/// <summary>
	/// 
	/// </summary>
	String Master::Attack(BasePlayer& opponent)
	{
		ZeroMemory(this->m_battleMessage, sizeof(this->m_battleMessage));
		// 处理异常状态
		
		if (this->InState(State::WEAKEN))
		{
			if (this->m_stateRoundsCnt.weaken = 1)
			{
				this->m_property.m_attack -= this->m_effects.weaken.attack;
				this->m_property.m_defense -= this->m_effects.weaken.defense;
				this->SubState(State::WEAKEN);
				sprintf(m_battleMessage + std::strlen(m_battleMessage), 
					"解除虚弱。");
			}
			else
			{
				--this->m_stateRoundsCnt.weaken;
			}
		}

		if (this->InState(State::SUNDERED))
		{
			if (this->m_stateRoundsCnt.sundered == 1)
			{
				this->m_property.m_attack -= this->m_effects.sundered.attack;
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

		bool hit      = false;
		bool critical = false;
		bool anger    = false;
		int  skill    = -1;
		if (_Hit_Target(this->m_property.m_hitratio, opponent.GetParryratio()))
		{	// 命中目标
			hit = true;
			Value iAT = this->m_property.m_attack;

			if (_Hit_Target(this->m_property.m_critical, opponent.GetCritical())) 
			{ // 暴击
				critical = true;
				iAT = static_cast<Value>((double)iAT * 1.5);
			}

			if (!this->InState(State::SILENT) && this->InState(State::ANGRIED))
			{
				anger = true;
				skill = 2;
				// 重设怒气值
				this->m_anger = 0;
				this->SubState(State::ANGRIED);

				if (this->m_property.m_hpoints / (this->m_angriedCnt * this->m_angriedCnt) == 0)
				{
					// 治愈系技能使用过度 强制使用攻击增益BUFF
					sprintf(m_battleMessage + std::strlen(m_battleMessage),
						"发动狂暴技能：强攻，增加%d点攻击力。", static_cast<Value>((double)this->m_property.m_attack / 20.0));
					switch (this->m_career.type)
					{
					case Career::Type::Normal:
					{
						this->m_property.m_attack += static_cast<Value>((double)this->m_property.m_attack / 20.0);
					}
					break;

					case Career::Type::GreatMasterOfLight:
					{
						this->m_property.m_attack += static_cast<Value>((double)this->m_property.m_attack / 50.0);
					}
					break;

					case Career::Type::GreatMasterOfDark:
					{
						this->m_property.m_attack += static_cast<Value>((double)this->m_property.m_attack / 10.0);
					}
					break;

					default:
						break;
					}
				}
				else
				{
					// 治愈系技能
					sprintf(m_battleMessage + std::strlen(m_battleMessage),
						"发动狂暴技能：死者苏生，恢复%d点生命值。", 
						static_cast<Value>((double)this->m_property.m_hpoints / ((double)this->m_angriedCnt * (double)this->m_angriedCnt)));
					this->m_property.m_hpoints = std::min<Value>(
						this->m_hpointsLimitation,
						this->m_property.m_hpoints + static_cast<Value>((double)this->m_property.m_hpoints / ((double)this->m_angriedCnt * (double)this->m_angriedCnt))
						);
					switch (this->m_career.type)
					{
					case Career::Type::Normal:
					{
						this->m_angriedCnt += 3;
					}
					break;

					case Career::Type::GreatMasterOfLight:
					{
						this->m_angriedCnt += 2;
					}
					break;

					case Career::Type::GreatMasterOfDark:
					{
						this->m_angriedCnt += 4;
					}
					break;

					default:
						break;
					}
				}

				if (!this->InState(State::WEAKEN))
				{
					// 狂暴后进入虚弱状态
					sprintf(m_battleMessage + std::strlen(m_battleMessage), "狂暴之后进入虚弱状态。");
					this->AddState(State::WEAKEN);

					this->m_effects.weaken.attack = ConvertValueByPercent(this->m_property.m_attack, this->m_skill.weakenIndex);
					this->m_effects.weaken.defense = ConvertValueByPercent(this->m_property.m_defense, this->m_skill.weakenIndex);
					this->m_property.m_attack += this->m_effects.weaken.attack;
					this->m_property.m_defense += this->m_effects.weaken.defense;

					this->SetWeakenRounds(CommonBasicValues::weakenRounds);
				}
			}
			else if (!this->InState(State::SILENT))
			{
				switch (this->m_skill.primarySkill)
				{
				case Skill::Type::RAGED:
				{
					if (_Hit_Target(this->m_skill.ragedChance, 0x0))
					{
						sprintf(m_battleMessage + std::strlen(m_battleMessage), 
							"发动技能：愤怒，使自身在下次被攻击时获得双倍的狂暴点。");

						skill = 0;
						this->AddState(State::RAGED);
					}
					else if (_Hit_Target(this->m_skill.selfHealingChance, 0x0))
					{
						sprintf(m_battleMessage + std::strlen(m_battleMessage), 
							"发动技能：治愈，恢复%d点生命值。",
							HealingHpointsCalculator(this->m_property.m_hpoints, this->m_skill.selfHealingIndex));

						skill = 1;
						this->m_skill.selfHealingChance = std::max<Value>((Value)5, this->m_skill.selfHealingChance - 1);
						this->m_skill.ragedChance = std::min<Value>((Value)30, this->m_skill.ragedChance + 1);

						this->m_property.m_hpoints = std::min<Value>(
							this->m_property.m_hpoints + HealingHpointsCalculator(this->m_property.m_hpoints, this->m_skill.selfHealingIndex),
							this->m_hpointsLimitation
							);
					}
				}
				break;

				case Skill::Type::SELF_HEALING:
				{
					if (_Hit_Target((Value)100, this->m_skill.selfHealingChance))
					{
						sprintf(m_battleMessage + std::strlen(m_battleMessage), 
							"发动技能：治愈，恢复%d点生命值。",
							HealingHpointsCalculator(this->m_property.m_hpoints, this->m_skill.selfHealingIndex));

						skill = 1;
						this->m_skill.selfHealingChance = std::max<Value>((Value)10, this->m_skill.selfHealingChance - 1);
						this->m_skill.ragedChance = std::min<Value>((Value)20, this->m_skill.ragedChance + 1);

						this->m_property.m_hpoints = std::min<Value>(
							this->m_property.m_hpoints + HealingHpointsCalculator(this->m_property.m_hpoints, this->m_skill.selfHealingIndex),
							this->m_hpointsLimitation
							);
					}
					else if (_Hit_Target((Value)100, this->m_skill.ragedChance))
					{
						sprintf(m_battleMessage + std::strlen(m_battleMessage),
							"发动技能：愤怒，使自身在下次被攻击时获得双倍的狂暴点。");

						skill = 0;
						this->AddState(State::RAGED);
					}
				}
				break;

				default:
					break;
				}

				// 攻击敌方小精灵
				sprintf(m_battleMessage + std::strlen(m_battleMessage),
					"造成%d点伤害。", 
					AttackDamageCalculator(iAT, opponent.GetDefense()));
				Value damage = opponent.IsAttacked(AttackDamageCalculator(iAT, opponent.GetDefense()));
				if (damage > 0)
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
				else if (this->m_career.type == Career::Type::GreatMasterOfDark)
				{
					this->m_anger = std::min<Value>(this->m_anger + Career::Darker::angerExtra, CommonBasicValues::angerLimitation);
					if (this->m_anger == CommonBasicValues::angerLimitation)
					{
						this->m_anger = 0;
						this->AddState(State::ANGRIED);
					}
				}
			}
		}
		else
		{
			sprintf(m_battleMessage + std::strlen(m_battleMessage), "未命中。");
		}

		return m_battleMessage;
	}

	/// <summary>
	/// 
	/// </summary>
	Value Master::IsAttacked(Value damage)
	{
		if (damage >= this->m_property.m_hpoints)
		{
			this->m_property.m_hpoints = 0;
			this->m_state = State::DEAD;
		}
		else
		{
			this->m_property.m_hpoints -= damage;
			if (this->InState(State::RAGED))
			{
				this->m_anger = std::min<Value>(CommonBasicValues::angerLimitation, this->m_anger + CommonBasicValues::angerInc * (Value)2);
				this->SubState(State::RAGED);
			}
			else
			{
				this->m_anger = std::min<Value>(CommonBasicValues::angerLimitation, this->m_anger + CommonBasicValues::angerInc);
			}

			if (this->m_anger == CommonBasicValues::angerLimitation)
				this->AddState(State::ANGRIED);

			if (this->InState(State::BLEED))
			{
				this->m_property.m_hpoints -= BloodingDamageCalculator(CommonBasicValues::bleedDamage, this->m_property.m_defense);
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

	bool Master::SetPrimarySkill(Skill::Type primarySkill)
	{
		this->m_skill.primarySkill = primarySkill;
		return false;
	}

	bool Master::Promote(Career::Type career)
	{
		if (this->m_career.type == Career::Type::Normal)
		{
			this->m_career.type = career;
			switch (this->m_career.type)
			{
			case Career::Type::GreatMasterOfLight:
			{
				this->m_skill.selfHealingIndex += ConvertValueByPercent(this->m_skill.selfHealingIndex, Career::Lighter::healingIncIndex);
				this->m_skill.selfHealingChance += ConvertValueByPercent(this->m_skill.selfHealingChance, Career::Lighter::selfHealingChanceIncIndex);
				this->m_property.m_attack += ConvertValueByPercent(this->m_property.m_attack, Career::Lighter::damageDecIndex);
				this->m_property.m_hpoints += ConvertValueByPercent(this->m_property.m_hpoints, Career::Lighter::hpointsIncIndex);
				this->m_skill.weakenIndex += ConvertValueByPercent(this->m_skill.weakenIndex, Career::Lighter::weakenDecIndex);
			}
			break;

			case Career::Type::GreatMasterOfDark:
			{
				this->m_skill.selfHealingChance += ConvertValueByPercent(this->m_skill.selfHealingChance, Career::Darker::selfHealingChanceDecIndex);
				this->m_property.m_defense += ConvertValueByPercent(this->m_property.m_defense, Career::Darker::defenseDecIndex);
				this->m_property.m_attack += ConvertValueByPercent(this->m_property.m_attack, Career::Darker::damageIncIndex);
				this->m_skill.weakenIndex += ConvertValueByPercent(this->m_skill.weakenIndex, Career::Darker::weakenIncIndex);
			}
			break;

			default:
				break;
			}
		}
		return false;
	}

	void Master::_LevelUpPropertiesDistributor_()
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
				+= static_cast<Value>((double)this->m_property.m_hpoints / 10.0) + CommonBasicValues::hpointsInc;
		}
		break;

		case Career::Type::GreatMasterOfLight:
		{
			defenseInc
				= _Random(static_cast<Value>(CommonBasicValues::levelupPropertiesInc / 2), CommonBasicValues::levelupPropertiesInc);
			attackInc
				= _Random(CommonBasicValues::levelupPropertiesInc - defenseInc);
			agilityInc
				= CommonBasicValues::levelupPropertiesInc - attackInc - defenseInc;

			this->m_property.m_hpoints 
				+= ConvertValueByPercent(static_cast<Value>((double)this->m_property.m_hpoints / 10.0), Career::Lighter::hpointsIncIndex) + CommonBasicValues::hpointsInc;
		}
		break;

		case Career::Type::GreatMasterOfDark:
		{
			attackInc
				= _Random(static_cast<Value>(CommonBasicValues::levelupPropertiesInc / 2), CommonBasicValues::levelupPropertiesInc);
			defenseInc 
				= _Random(CommonBasicValues::levelupPropertiesInc - attackInc);
			agilityInc 
				= CommonBasicValues::levelupPropertiesInc - attackInc - defenseInc;
			attackInc
				+= ConvertValueByPercent(attackInc, Career::Darker::damageIncIndex);

			this->m_property.m_hpoints 
				+= static_cast<Value>((double)this->m_property.m_hpoints / 20.0) + CommonBasicValues::hpointsInc;
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