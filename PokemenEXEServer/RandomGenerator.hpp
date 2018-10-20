#pragma once

#include <random>

namespace Pokemen
{
	/// <summary>
	/// 
	/// </summary>
	static std::default_random_engine g_Rand_Engine;
	template <class _Ty>
	static _Ty _Random(_Ty limit)
	{
		std::random_device rd;
		g_Rand_Engine.seed(rd());

		std::uniform_int_distribution<> u(0, limit - 1);
		return u(g_Rand_Engine);
	}

	/// <summary>
	/// 
	/// </summary>
	template <class _Ty>
	static _Ty _Random(_Ty min, _Ty max)
	{
		std::random_device rd;
		g_Rand_Engine.seed(rd());

		std::uniform_int_distribution<> u(min, max);
		return u(g_Rand_Engine);
	}

	/// <summary>
	/// 
	/// </summary>
	static bool _Hit_Target(int16_t limit, int16_t threshold)
	{
		std::random_device rd;
		g_Rand_Engine.seed(rd());

		std::uniform_int_distribution<> u(0, limit);
		if (u(g_Rand_Engine) < threshold)
			return true;
		else
			return false;
	}
}