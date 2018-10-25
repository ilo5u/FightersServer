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
		// �����쳣״̬
		if (this->InState(State::WEAKEN))
		{
			if (this->m_weakenRoundsCnt == 1)
			{
				this->m_attack = this->m_attackOriginal;
				this->SubState(State::WEAKEN);
				sprintf(battleMessage + std::strlen(battleMessage), "���������");
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
				sprintf(battleMessage + std::strlen(battleMessage), "�����ָ���");
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
				sprintf(battleMessage + std::strlen(battleMessage), "���ѣ�Ρ�");
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
		{	// ����Ŀ��
			hit = true;
			Value iAT = this->m_attack;

			if (_Hit_Target((Value)100, this->m_critical))
			{	// ����
				critical = true;
				iAT = static_cast<Value>((double)iAT * 1.5);
			}

			if (!this->InState(State::SILENT) && this->InState(State::ANGRIED))
			{
				anger = true;
				skill = 2;
				// ����ŭ��ֵ
				this->m_anger = 0;
				this->SubState(State::ANGRIED);

				if (this->m_hpoints / (this->m_angriedCnt * this->m_angriedCnt) == 0)
				{
					// ����ϵ����ʹ�ù��� ǿ��ʹ�ù�������BUFF
					sprintf(battleMessage + std::strlen(battleMessage),
						"�����񱩼��ܣ�ǿ��������%d�㹥������", static_cast<Value>((double)this->m_attack / 20.0));
					this->m_attack += static_cast<Value>((double)this->m_attack / 20.0);
				}
				else
				{
					// ����ϵ����
					sprintf(battleMessage + std::strlen(battleMessage),
						"�����񱩼��ܣ������������ָ�%d������ֵ��", 
						static_cast<Value>((double)this->m_hpoints / ((double)this->m_angriedCnt * (double)this->m_angriedCnt)));
					this->m_hpoints = std::min<Value>(
						this->m_hpointsLimitation,
						this->m_hpoints + static_cast<Value>((double)this->m_hpoints / ((double)this->m_angriedCnt * (double)this->m_angriedCnt))
						);
					this->m_angriedCnt += 2;
				}

				if (!this->InState(State::WEAKEN))
				{
					// �񱩺��������״̬
					sprintf(battleMessage + std::strlen(battleMessage), "��֮���������״̬��");

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
						sprintf(battleMessage + std::strlen(battleMessage), "�������ܣ���ŭ��ʹ�������´α�����ʱ���˫���Ŀ񱩵㡣");

						skill = 0;
						this->AddState(State::DBANGRY);
					}
					else if (_Hit_Target((Value)100, this->m_skill.m_selfHealingProbability))
					{
						sprintf(battleMessage + std::strlen(battleMessage), "�������ܣ��������ָ�%d������ֵ��",
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
						sprintf(battleMessage + std::strlen(battleMessage), "�������ܣ��������ָ�%d������ֵ��",
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
						sprintf(battleMessage + std::strlen(battleMessage), "�������ܣ���ŭ��ʹ�������´α�����ʱ���˫���Ŀ񱩵㡣");

						skill = 0;
						this->AddState(State::DBANGRY);
					}
				}
				break;

				default:
					break;
				}

				// �����з�С����
				sprintf(battleMessage + std::strlen(battleMessage), "���%d���˺���", 
					AttackDamageCalculator(iAT, opponent->GetDefense()));
				Value damage = opponent->IsAttacked(AttackDamageCalculator(iAT, opponent->GetDefense()));
				if (damage > 0)
				{	// �Է���������
					sprintf(battleMessage + std::strlen(battleMessage), "�ܵ�%d�㷴�ˡ�", damage);
					this->m_hpoints -= damage;
				}

				if (this->m_hpoints <= 0)
				{
					sprintf(battleMessage + std::strlen(battleMessage), "С����������");
					this->m_hpoints = 0;
					this->m_state = State::DEAD;
				}
			}
		}
		else
		{
			sprintf(battleMessage + std::strlen(battleMessage), "δ���С�");
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