#pragma once

#define clamp(val, _min, _max) max(min(val, _max), _min);
#define clamp01(val) clamp(val, 0, 1)

class Mathf
{
public:

	template<class T>
	inline static T Lerp(T from, T to, float t)
	{
		return from + (to - from) * t;
	}
};