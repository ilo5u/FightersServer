#include "stdafx.h"

namespace Pokemen
{
	static const Strings namesOfTank{ "Bulbasaur", "Ivysaur", "Venusaur", "Charmander" };
	static char  battleMessage[BUFLEN];

	/// <summary>
	/// 
	/// </summary>
	Tank::Tank(Skill::Type primarySkill, int level) :
		BasePlayer(
			namesOfTank[_Random(namesOfTank.size())],
			hitpointsBase + _Random(propertyOffset),
			attackBase + _Random(propertyOffset),
			defenseBase + _Random(propertyOffset),
			agilityBase + _Random(propertyOffset),
			PokemenType::TANK
		),
		m_skill(primarySkill)
	{
		while (this->m_level < std::min<Value>(level, levelLimitation))
			Update(expBase);
	}

	/// <summary>
	/// 
	/// </summary>
	Tank::~Tank()
	{
	}

	Tank::Skill::Skill(Type primarySkill)
	{
		m_primarySkill = primarySkill;
	}

	/// <summary>
	/// 
	/// </summary>
	String Tank::Attack(HCPokemenPlayer opponent)
	{
		ZeroMemory(battleMessage, sizeof(battleMessage));
		// 处理异常状态
		if (this->InState(State::WEAKEN))
		{
			if (this->m_weakenRoundsCnt == 1)
			{
				this->m_attack = this->m_attackOriginal;
				this->SubState(State::WEAKEN);
				sprintf(battleMessage + std::strlen(battleMessage), "解除虚弱。");
			}
			else
			{
				--this->m_weakenRoundsCnt;
			}
		}

		if (this->InState(State::SUNDERED))
		{
			if (this->m_sunderedRoundsCnt == 1)
			{
				this->m_attack = this->m_attackOriginal;
				this->SubState(State::SUNDERED);
				sprintf(battleMessage + std::strlen(battleMessage), "武器恢复。");
			}
			else
			{
				--this->m_dizzyRoundsCnt;
			}
		}

		if (this->InState(State::DIZZYING))
		{
			if (this->m_dizzyRoundsCnt == 1)
			{
				this->SubState(State::DIZZYING);
				sprintf(battleMessage + std::strlen(battleMessage), "解除眩晕。");
			}
			else
			{
				--this->m_dizzyRoundsCnt;
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
		if (_Hit_Target((Value)100,
			(Value)_Random(10) + (Value)10 * (Value)std::sqrt(std::max<double>(double(this->m_hitratio - this->m_parryratio), 0.0))))
		{	// 命中目标
			hit = true;
			Value iAT = this->m_attack;

			if (_Hit_Target((Value)100, this->m_critical))
			{	// 暴击
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

				if (this->m_hpoints / (this->m_angriedCnt * this->m_angriedCnt) == 0)
				{
					// 治愈系技能使用过度 强制使用攻击增益BUFF
					sprintf(battleMessage + std::strlen(battleMessage),
						"发动狂暴技能：强攻，增加%d点攻击力。", static_cast<Value>((double)this->m_attack / 20.0));
					this->m_attack += static_cast<Value>((double)this->m_attack / 20.0);
				}
				else
				{
					// 治愈系技能
					sprintf(battleMessage + std::strlen(battleMessage),
						"发动狂暴技能：死者苏生，恢复%d点生命值。", 
						static_cast<Value>((double)this->m_hpoints / ((double)this->m_angriedCnt * (double)this->m_angriedCnt)));
					this->m_hpoints = std::min<Value>(
						this->m_hpointsLimitation,
						this->m_hpoints + static_cast<Value>((double)this->m_hpoints / ((double)this->m_angriedCnt * (double)this->m_angriedCnt))
						);
					this->m_angriedCnt += 2;
				}

				if (!this->InState(State::WEAKEN))
				{
					// 狂暴后进入虚弱状态
					sprintf(battleMessage + std::strlen(battleMessage), "狂暴之后进入虚弱状态。");

					this->AddState(State::WEAKEN);
					this->m_attackOriginal = this->m_attack;
					this->m_attack = static_cast<Value>((double)iAT * ((double)this->m_skill.m_weakenIndex / 100.0));
					this->m_weakenRoundsCnt = this->weakenRoundsBase;
				}
			}
			else if (!this->InState(State::SILENT))
			{
				switch (this->m_skill.m_primarySkill)
				{
				case Skill::Type::DOUBLE_ANGRY:
				{
					if (_Hit_Target((Value)100, this->m_skill.m_doubleAngryProbability))
					{
						sprintf(battleMessage + std::strlen(battleMessage), "发动技能：愤怒，使自身在下次被攻击时获得双倍的狂暴点。");

						skill = 0;
						this->AddState(State::DBANGRY);
					}
					else if (_Hit_Target((Value)100, this->m_skill.m_selfHealingProbability))
					{
						sprintf(battleMessage + std::strlen(battleMessage), "发动技能：治愈，恢复%d点生命值。",
							HealingHpointsCalculator(this->m_hpoints, this->m_skill.m_selfHealingIndex));

						skill = 1;
						this->m_skill.m_selfHealingProbability = std::max<Value>((Value)5, this->m_skill.m_selfHealingProbability - 1);
						this->m_skill.m_doubleAngryProbability = std::min<Value>((Value)30, this->m_skill.m_doubleAngryProbability + 1);

						this->m_hpoints = std::min<Value>(
							this->m_hpoints + HealingHpointsCalculator(this->m_hpoints, this->m_skill.m_selfHealingIndex),
							this->m_hpointsLimitation
							);
					}
				}
				break;

				case Skill::Type::SELF_HEALING:
				{
					if (_Hit_Target((Value)100, this->m_skill.m_selfHealingProbability))
					{
						sprintf(battleMessage + std::strlen(battleMessage), "发动技能：治愈，恢复%d点生命值。",
							HealingHpointsCalculator(this->m_hpoints, this->m_skill.m_selfHealingIndex));

						skill = 1;
						this->m_skill.m_selfHealingProbability = std::max<Value>((Value)10, this->m_skill.m_selfHealingProbability - 1);
						this->m_skill.m_doubleAngryProbability = std::min<Value>((Value)20, this->m_skill.m_doubleAngryProbability + 1);

						this->m_hpoints = std::min<Value>(
							this->m_hpoints + HealingHpointsCalculator(this->m_hpoints, this->m_skill.m_selfHealingIndex),
							this->m_hpointsLimitation
							);
					}
					else if (_Hit_Target((Value)100, this->m_skill.m_doubleAngryProbability))
					{
						sprintf(battleMessage + std::strlen(battleMessage), "发动技能：愤怒，使自身在下次被攻击时获得双倍的狂暴点。");

						skill = 0;
						this->AddState(State::DBANGRY);
					}
				}
				break;

				default:
					break;
				}

				// 攻击敌方小精灵
				sprintf(battleMessage + std::strlen(battleMessage), "造成%d点伤害。", 
					AttackDamageCalculator(iAT, opponent->GetDefense()));
				Value damage = opponent->IsAttacked(AttackDamageCalculator(iAT, opponent->GetDefense()));
				if (damage > 0)
				{	// 对方开启反甲
					sprintf(battleMessage + std::strlen(battleMessage), "受到%d点反伤。", damage);
					this->m_hpoints -= damage;
				}

				if (this->m_hpoints <= 0)
				{
					sprintf(battleMessage + std::strlen(battleMessage), "小精灵死亡。");
					this->m_hpoints = 0;
					this->m_state = State::DEAD;
				}
			}
		}
		else
		{
			sprintf(battleMessage + std::strlen(battleMessage), "未命中。");
		}

		sprintf(battleMessage + std::strlen(battleMessage),
			"\nHI=%d,CR=%d,AN=%d,SK=%d,ST=%d\n", hit, critical, anger, skill, this->m_state);
		return battleMessage;
	}

	/// <summary>
	/// 
	/// </summary>
	Value Tank::IsAttacked(Value damage)
	{
		if (damage >= this->m_hpoints)
		{
			this->m_hpoints = 0;
			this->m_state = State::DEAD;
		}
		else
		{
			this->m_hpoints -= damage;
			if (this->InState(State::DBANGRY))
			{
				this->m_anger = std::min<Value>(angerLimitation, this->m_anger + angerIncrementBase * (Value)2);
				this->SubState(State::DBANGRY);
			}
			else
			{
				this->m_anger = std::min<Value>(angerLimitation, this->m_anger + angerIncrementBase);
			}

			if (this->m_anger == angerLimitation)
				this->AddState(State::ANGRIED);

			if (this->InState(State::BLOODING))
			{
				this->m_hpoints -= BloodingDamageCalculator(bloodingDamageBase, this->m_defense);
				if (this->m_hpoints <= 0)
				{
					this->m_hpoints = 0;
					this->m_state = State::DEAD;
				}

				if (this->m_bloodingRoundsCnt == 1)
					this->SubState(State::BLOODING);
			}
		}
		return 0;
	}

	bool Tank::SetPrimarySkill(Skill::Type primarySkill)
	{
		this->m_skill.m_primarySkill = primarySkill;
		return false;
	}

	void Tank::_LevelUpPropertiesDistributor_()
	{
		Value attack_increment  = _Random(levelupPropertiesIncrementBase / 2);
		Value defense_increment = _Random(levelupPropertiesIncrementBase / 2);
		Value agility_increment = levelupPropertiesIncrementBase - attack_increment - defense_increment;

		this->m_hpoints += static_cast<Value>((double)this->m_hpoints / 5.0) + hpointsLevelupBase;
		this->m_attack  += attack_increment;
		this->m_defense += defense_increment;
		this->m_agility += agility_increment;
	}
}