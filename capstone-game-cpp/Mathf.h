#pragma once
#include <Rig3d/Parametric.h>
#include <random>

#define clamp(val, _min, _max) max(min((val), (_max)), (_min));
#define clamp01(val) clamp((val), 0, 1)
#define PI_TWO PI * 2

class Mathf
{
	inline static std::mt19937& __RandomGenerator()
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		return gen;
	}

public:

	/// Produces random floating-point values i, uniformly distributed on the interval [a, b)
	inline static float RandomRange(float min = 0, float max = 1)
	{
		std::uniform_real_distribution<float> dis(min, max);
		return dis(__RandomGenerator());
	}

	/// Produces random floating-point values i, uniformly distributed on the interval [a, b)
	inline static int RandomRangeInt(int min = 0, int max = 10)
	{
		std::uniform_int_distribution<int> dis(min, max - 1);
		return dis(__RandomGenerator());
	}

	inline static float Repeat(float t, float length)
	{
		return t - floor(t / length) * length;
	}

	inline static float LerpAngle(float a, float b, float t)
	{
		float num = Repeat(b - a, PI_TWO);
		if (num > PI)
		{
			num -= PI_TWO;
		}
		return a + num * clamp01(t);
	}

	inline static float MoveTowards(float current, float target, float maxDelta)
	{
		if (abs(target - current) <= maxDelta)
		{
			return target;
		}
		return current + Sign(target - current) * maxDelta;
	}

	inline static float Sign(float f)
	{
		return (f < 0.0f) ? -1.0f : 1.0f;
	}

	inline static float MoveTowardsAngle(float current, float target, float maxDelta)
	{
		target = current + DeltaAngle(current, target);
		return MoveTowards(current, target, maxDelta);
	}

	inline  static float DeltaAngle(float current, float target)
	{
		float num = Repeat(target - current, PI_TWO);
		if (num > PI)
		{
			num -= PI_TWO;
		}
		return num;
	}

	template<class VectorType>
	inline static VectorType Lerp(const VectorType& from, const VectorType& to, float t)
	{
		return from + (to - from) * t;
	}

	template<class VectorType>
	inline static VectorType Clamp(const VectorType& vector, const AABB<VectorType>& bounds)
	{
		int numElements = sizeof(VectorType) / sizeof(float);
		return Clamp(vector, bounds.origin - bounds.halfSize, bounds.origin + bounds.halfSize);
	}

	template<class VectorType>
	inline static VectorType Clamp(const VectorType& vector, VectorType minBounds, VectorType maxBounds)
	{
		int numElements = sizeof(VectorType) / sizeof(float);

		VectorType v(vector);
		for (int i = 0; i < numElements; i++)
		{
			v[i] = clamp(v[i], minBounds[i], maxBounds[i]);
		}

		return v;
	}

	template<class VectorType>
	inline static VectorType Clamp01(const VectorType& vector)
	{
		int numElements = sizeof(VectorType) / sizeof(float);

		VectorType v(vector);
		for (int i = 0; i < numElements; i++)
		{
			v[i] = clamp01(v[i]);
		}

		return v;
	}



	template<class VectorType>
	inline static AABB<VectorType> Fit(AABB<VectorType> inner, AABB<VectorType> outer)
	{
		int numElements = sizeof(VectorType) / sizeof(float);

		AABB<VectorType> result;
		//result.origin = outer.origin - originOffset;

		for (int i = 0; i < numElements; i++)
		{
			// move origin to outer origin
			result.origin[i] = outer.origin[i];

			// shrink size to fit
			result.halfSize[i] = min(inner.halfSize[i], outer.halfSize[i]);

			// extentsoffset is positive whenever inner bounds is smaller than outer bounds
			float extentsOffset = outer.halfSize[i] - inner.halfSize[i];
			if (extentsOffset >= 0)
			{ 
				result.origin[i] -= clamp(outer.origin[i] - inner.origin[i], -extentsOffset, extentsOffset);
			}
		}

		return result;
	}

};