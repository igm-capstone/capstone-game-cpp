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
			union
			{
				struct
				{
					float x, y, z, w;
				};

				struct
				{
					Vector3 v;
					float padding;
				};

				float pCols[4];
			};

			static Quaternion rollPitchYaw(const float& roll, const float& pitch, const float& yaw);
			static Quaternion angleAxis(const float& angle, const Vector3& axis);
			static Quaternion fromMatrix3(const Matrix3& mat);

			Quaternion(const float& w, const float& x, const float& y, const float& z) : x(x), y(y), z(z), w(w) {};
			Quaternion(const float& w, const Vector3& v) : Quaternion(w, v.x, v.y, v.z) {};
			Quaternion(const Quaternion& quaternion) : Quaternion(quaternion.w, quaternion.v) {};
			Quaternion(const float& v) : Quaternion(0.0f, 0.0f, 0.0f, 0.0f) {};
			Quaternion() : Quaternion(1.0f, 0.0f, 0.0f, 0.0f) {};

			Quaternion conjugate() const;
			Quaternion inverse() const;

			Matrix4		toMatrix4() const;
			Matrix3		toMatrix3() const;
			Vector3		toEuler() const;
			void		toAngleAxis(float* outAngle, Vector3* outAxis) const;

			Quaternion& operator+=(const Quaternion& rhs);
			Quaternion& operator*=(const Quaternion& rhs);
			Quaternion& operator*=(const float& rhs);
			Quaternion& operator/=(const float& rhs);
			Quaternion& operator=(const Quaternion& rhs);
			Quaternion operator-();
		};

		CGM_DLL Quaternion slerp(Quaternion q0, Quaternion q1, const float& t);

		CGM_DLL bool operator==(const Quaternion& lhs, const Quaternion& rhs);
		CGM_DLL bool operator!=(const Quaternion& lhs, const Quaternion& rhs);

		CGM_DLL Quaternion operator+(const Quaternion& lhs, const Quaternion& rhs);
		CGM_DLL Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs);

		CGM_DLL Vector3 operator*(const Vector3& lhs, const Quaternion& rhs);
		CGM_DLL Vector3 operator*(const Quaternion& lhs, const Vector3& rhs);

		CGM_DLL Quaternion operator*(const Quaternion& lhs, const float& rhs);
		CGM_DLL Quaternion operator*(const float& lhs, const Quaternion& rhs);

		CGM_DLL Quaternion operator/(const Quaternion& lhs, const float& rhs);
	}
}