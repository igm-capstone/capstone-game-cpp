#pragma once

class Mathf
{
public:

	template<class T>
	static T Lerp(T from, T to, float t)
	{
		return from + (to - from) * t;
	}
};