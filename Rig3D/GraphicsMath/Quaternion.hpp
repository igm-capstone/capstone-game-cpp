//	Quaternion.hpp
//	Gabriel Ortega
//
//	

#pragma once
#include "Matrix.hpp"

#ifdef _WINDLL
#define CGM_DLL __declspec(dllexport)
#else
#define CGM_DLL __declspec(dllimport)
#endif

namespace cliqCity
{
	namespace graphicsMath
	{
		struct CGM_DLL Quaternion
		{
			Vector3 v;
			float w;

			static Quaternion rollPitchYaw(const float& roll, const float& pitch, const float& yaw);
			static Quaternion angleAxis(const float& angle, const Vector3& axis);

			Quaternion(const float& w, const Vector3& v) : w(w), v(v) {};
			Quaternion(const float& w, const float& x, const float& y, const float& z) : w(w), v(x, y, z) {};
			Quaternion(const Quaternion& quaternion) : Quaternion(quaternion.w, quaternion.v) {};
			Quaternion() : Quaternion(1.0f, 0.0f, 0.0f, 0.0f) {};

			float magnitude() const;
			Quaternion conjugate() const;
			Quaternion inverse() const;

			Matrix4 toMatrix4() const;
			Matrix3 toMatrix3() const;

			Quaternion& operator*=(const Quaternion& rhs);
			Quaternion& operator*=(const float& rhs);
			Quaternion& operator/=(const float& rhs);
			Quaternion& operator-();
		};

		CGM_DLL Quaternion normalize(const Quaternion& quaternion);
		CGM_DLL float dot(const Quaternion& lhs, const Quaternion& rhs);

		CGM_DLL Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs);
		CGM_DLL Quaternion operator*(const Quaternion& lhs, const float& rhs);
		CGM_DLL Vector3 operator*(const Vector3& lhs, const Quaternion& rhs);
		CGM_DLL Vector3 operator*(const Quaternion& lhs, const Vector3& rhs);

		CGM_DLL Quaternion operator/(const Quaternion& lhs, const float& rhs);
	}
}