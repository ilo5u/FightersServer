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

		std::uniform_int_distribution<> u(0, (int)limit - 1);
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
	static bool _Hit_Target(int32_t positiveAffect, int32_t negativAffect)
	{
		std::random_device rd;
		g_Rand_Engine.seed(rd());

		std::uniform_int_distribution<> u(0, 100);
		if (u(g_Rand_Engine) 
			< (int32_t)_Random(10) + (int32_t)(10.0 * std::sqrt(std::max<double>(double(positiveAffect - negativAffect / 4.0), 0.0))))
			return true;
		else
			return false;
	}
}