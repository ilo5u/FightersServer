#include "stdafx.h"

namespace Pokemen
{
	static std::vector<std::string> g_Tankname_Base{ "Bulbasaur", "Ivysaur", "Venusaur", "Charmander" };
	static char g_wszMessage[1024];

	/// <summary>
	/// 
	/// </summary>
	Tank::Tank(Skill::Type primary_skill, int level) :
		BasePlayer(
			g_Tankname_Base[_Random(g_Tankname_Base.size())],
			hitpoints_base + _Random(property_offset),
			attack_base + _Random(property_offset),
			defense_base + _Random(property_offset),
			agility_base + _Random(property_offset),
			PokemenType::TANK
		),
		m_skill(primary_skill)
	{
		while (this->m_level < min(level, level_limitation))
			Update(exp_base);
	}

	/// <summary>
	/// 
	/// </summary>
	Tank::~Tank()
	{
	}

	Tank::Skill::Skill(Type primary_skill)
	{
		m_primary_skill = primary_skill;
	}

	/// <summary>
	/// 
	/// </summary>
	std::string Tank::Attack(HCPokemenPlayer opponent)
	{
		std::memset(g_wszMessage, 0x0, sizeof(g_wszMessage));
		// weaken
		if (this->InState(State::WEAKEN))
		{
			if (this->m_weaken_circles_cnt == 1)
			{
				this->m_attack = this->m_attack_original;
				this->SubState(State::WEAKEN);
				std::sprintf(g_wszMessage + std::strlen(g_wszMessage), "Released from weaken state. ");
			}
			else
			{
				--this->m_weaken_circles_cnt;
			}
		}

		if (this->InState(State::DIZZYING)
			|| this->InState(State::SILENT) 
			|| this->InState(State::DEAD))
		{
			std::sprintf(g_wszMessage + std::strlen(g_wszMessage), "Give up. ");
			return false;
		}

		// can or can not hit the target
		if (_Hit_Target((int16_t)100,
			(int16_t)_Random(10) + (int16_t)10 * (int16_t)std::sqrt(max(double(this->m_hitratio - this->m_parryratio), 0.0))))
		{	// hit the target
			int16_t iAT = this->m_attack;

			// critical chance
			if (_Hit_Target((int16_t)100, this->m_critical))
			{
				std::sprintf(g_wszMessage + std::strlen(g_wszMessage), "Got a critical-chance. ");
				iAT = iAT + iAT / 2;
			}

			if (this->InState(State::ANGRIED))
			{
				std::sprintf(g_wszMessage + std::strlen(g_wszMessage), "Be angried. ");
				// release
				this->m_anger = 0;
				this->SubState(State::ANGRIED);

				if (this->m_hpoints / (this->m_angried_cnt * this->m_angried_cnt) == 0)
				{
					std::sprintf(g_wszMessage + std::strlen(g_wszMessage), "Increase the attack value. ");
					this->m_attack += 3;
				}
				else
				{
					// BUFF
					std::sprintf(g_wszMessage + std::strlen(g_wszMessage), "Healing %d HP. ", std::min<int16_t>(
						this->m_hpoints_limitation,
						this->m_hpoints + this->m_hpoints / (this->m_angried_cnt * this->m_angried_cnt)
						) - this->m_hpoints);

					this->m_hpoints = std::min<int16_t>(
						this->m_hpoints_limitation,
						this->m_hpoints + this->m_hpoints / (this->m_angried_cnt * this->m_angried_cnt)
						);
					this->m_angried_cnt += 2;
				}

				if (!this->InState(State::WEAKEN))
				{
					// weaken
					std::sprintf(g_wszMessage + std::strlen(g_wszMessage), "Be weaken. ");

					this->AddState(State::WEAKEN);
					this->m_attack_original = this->m_attack;
					this->m_attack = static_cast<int16_t>((double)iAT * ((double)this->m_skill.m_weaken_index / 100.0));
					this->m_weaken_circles_cnt = this->weaken_circles_base;
				}
			}
			else
			{
				switch (this->m_skill.m_primary_skill)
				{
				case Skill::Type::DOUBLE_ANGRY:
				{
					if (_Hit_Target((int16_t)100, this->m_skill.m_double_angry_probability))
					{
						std::sprintf(g_wszMessage + std::strlen(g_wszMessage), "Got buff -- double angry. ");
						this->AddState(State::DBANGRY);
					}
					else if (_Hit_Target((int16_t)100, this->m_skill.m_self_healing_probability))
					{
						std::sprintf(g_wszMessage + std::strlen(g_wszMessage), "Healing %d HP. ", std::min<int16_t>(
							this->m_hpoints + HealingHpointsCalculator(this->m_hpoints, this->m_skill.m_self_healing_index),
							this->m_hpoints_limitation
							) - this->m_hpoints);

						this->m_skill.m_self_healing_probability = max((uint16_t)5, this->m_skill.m_self_healing_probability - 1);
						this->m_skill.m_double_angry_probability = min((uint16_t)30, this->m_skill.m_double_angry_probability + 1);

						this->m_hpoints = std::min<int16_t>(
							this->m_hpoints + HealingHpointsCalculator(this->m_hpoints, this->m_skill.m_self_healing_index),
							this->m_hpoints_limitation
							);
					}
				}
				break;

				case Skill::Type::SELF_HEALING:
				{
					if (_Hit_Target((int16_t)100, this->m_skill.m_self_healing_probability))
					{
						std::sprintf(g_wszMessage + std::strlen(g_wszMessage), "Healing %d HP. ", std::min<int16_t>(
							this->m_hpoints + HealingHpointsCalculator(this->m_hpoints, this->m_skill.m_self_healing_index),
							this->m_hpoints_limitation
							) - this->m_hpoints);

						this->m_skill.m_self_healing_probability = max((uint16_t)10, this->m_skill.m_self_healing_probability - 1);
						this->m_skill.m_double_angry_probability = min((uint16_t)20, this->m_skill.m_double_angry_probability + 1);

						this->m_hpoints = std::min<int16_t>(
							this->m_hpoints + HealingHpointsCalculator(this->m_hpoints, this->m_skill.m_self_healing_index),
							this->m_hpoints_limitation
							);
					}
					else if (_Hit_Target((int16_t)100, this->m_skill.m_double_angry_probability))
					{
						std::sprintf(g_wszMessage + std::strlen(g_wszMessage), "Got buff -- double angry. ");
						this->AddState(State::DBANGRY);
					}
				}
				break;

				default:
					break;
				}

				// Attack
				std::sprintf(g_wszMessage + std::strlen(g_wszMessage), "Cause %d damage. ", AttackDamageCalculator(iAT, opponent->GetDefense()));
				this->m_hpoints -= opponent->IsAttacked(AttackDamageCalculator(iAT, opponent->GetDefense()));
				if (this->m_hpoints <= 0)
				{
					std::sprintf(g_wszMessage + std::strlen(g_wszMessage), "Be dead!!! ");
					this->m_hpoints = 0;
					this->m_state = State::DEAD;
				}
			}
		}
		else
		{
			std::sprintf(g_wszMessage + std::strlen(g_wszMessage), "Missed. ");
		}
		return g_wszMessage;
	}

	/// <summary>
	/// 
	/// </summary>
	int16_t Tank::IsAttacked(int16_t damage)
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
				this->m_anger = std::min<int16_t>(anger_limitation_base, this->m_anger + anger_increase_base * (int16_t)2);
				this->SubState(State::DBANGRY);
			}
			else
			{
				this->m_anger = std::min<int16_t>(anger_limitation_base, this->m_anger + anger_increase_base);
			}

			if (this->m_anger == anger_limitation_base)
				this->AddState(State::ANGRIED);

			if (this->InState(State::BLOODING))
			{
				this->m_hpoints -= BloodingDamageCalculator(blooding_damage_base, this->m_defense);
				if (this->m_hpoints <= 0)
				{
					this->m_hpoints = 0;
					this->m_state = State::DEAD;
				}

				if (this->m_blooding_circles_cnt == 1)
					this->SubState(State::BLOODING);
			}
		}
		return 0;
	}

	bool Tank::SetPrimarySkill(Skill::Type primary_skill)
	{
		this->m_skill.m_primary_skill = primary_skill;
		return false;
	}

	void Tank::_level_up_property_distribution()
	{
		int16_t attack_increment = _Random(levelup_property_increased / 2);
		int16_t defense_increment = _Random(levelup_property_increased / 2);
		int16_t agility_increment = levelup_property_increased - attack_increment - defense_increment;

		this->m_hpoints += (this->m_hpoints / 4 + hpoints_levelup_base);
		this->m_attack += attack_increment;
		this->m_defense += defense_increment;
		this->m_agility += agility_increment;
	}
}