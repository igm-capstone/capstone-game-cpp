#pragma once
#include "SIMD/SIMD/simd.h"

#ifdef _WINDLL
#define CGM_DLL __declspec(dllexport)
#else
#define CGM_DLL __declspec(dllimport)
#endif

namespace cliqCity
{
	namespace graphicsMath
	{
		struct Vector4;
		struct Vector3;
		struct Vector2;

		struct __declspec(align(16)) CGM_DLL SIMDVector
		{
			union 
			{
				float128_t m;

				struct 
				{
					float x, y, z, w;
				};
			};

			static SIMDVector SetVector(const float& x, const float& y, const float& z, const float& w);
			static SIMDVector SetVector(const Vector4& vector);
			static SIMDVector SetVector(const Vector3& vector);
			static SIMDVector SetVector(const Vector2& vector);

			inline SIMDVector(const SIMDVector& other) : m(other.m) {};
			inline SIMDVector(const float128_t& m) : m(m) {};
			inline SIMDVector(const float& x, const float& y, const float& z, const float& w) : SIMDVector(simd::Set(x, y, z, w)){}
			inline SIMDVector() : SIMDVector(simd::Set(0.0f)) {};

			SIMDVector& operator+=(const SIMDVector& rhs);
			SIMDVector& operator-=(const SIMDVector& rhs);
			SIMDVector& operator*=(const SIMDVector& rhs);

			SIMDVector& operator+=(const float& rhs);
			SIMDVector& operator-=(const float& rhs);
			SIMDVector& operator*=(const float& rhs);
			SIMDVector& operator/=(const float& rhs);

			// Unary

			SIMDVector& operator=(const SIMDVector& rhs);
			SIMDVector operator-();

			float& operator[](const unsigned int& index);

			inline operator float128_arg_t() const
			{
				return m;
			}
		};

		CGM_DLL SIMDVector cross(const SIMDVector& lhs, const SIMDVector& rhs);
		CGM_DLL SIMDVector normalize(const SIMDVector& vector);
		CGM_DLL float magnitude(const SIMDVector& vector);
		CGM_DLL float magnitudeSquared(const SIMDVector& vector);
		CGM_DLL float dot(const SIMDVector& lhs, const SIMDVector& rhs);

		CGM_DLL SIMDVector operator+(const SIMDVector& lhs, const SIMDVector& rhs);
		CGM_DLL SIMDVector operator-(const SIMDVector& lhs, const SIMDVector& rhs);
		CGM_DLL SIMDVector operator*(const SIMDVector& lhs, const SIMDVector& rhs);

		CGM_DLL SIMDVector operator+(const SIMDVector& lhs, const float& rhs);
		CGM_DLL SIMDVector operator-(const SIMDVector& lhs, const float& rhs);
		CGM_DLL SIMDVector operator*(const SIMDVector& lhs, const float& rhs);
		CGM_DLL SIMDVector operator/(const SIMDVector& lhs, const float& rhs);

		CGM_DLL SIMDVector operator+(const float& lhs, const SIMDVector& rhs);
		CGM_DLL SIMDVector operator-(const float& lhs, const SIMDVector& rhs);
		CGM_DLL SIMDVector operator*(const float& lhs, const SIMDVector& rhs);
	}
}
