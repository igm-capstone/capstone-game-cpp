#define _USE_MATH_DEFINES
#include <cmath>
#include "Quaternion.hpp"

using namespace cliqCity::graphicsMath;

Quaternion Quaternion::rollPitchYaw(const float& roll, const float& pitch, const float& yaw)
{
	float halfRoll		= roll	* 0.5f;
	float halfPitch		= pitch * 0.5f;
	float halfYaw		= yaw	* 0.5f;
	float cosHalfRoll	= cosf(halfRoll);
	float cosHalfPitch	= cosf(halfPitch);
	float cosHalfYaw	= cosf(halfYaw);
	float sinHalfRoll	= sinf(halfRoll);
	float sinHalfPitch	= sinf(halfPitch);
	float sinHalfYaw	= sinf(halfYaw);
	return Quaternion(
		(cosHalfYaw * cosHalfPitch * cosHalfRoll) + (sinHalfYaw * sinHalfPitch * sinHalfRoll),
		(cosHalfYaw * sinHalfPitch * cosHalfRoll) + (sinHalfYaw * cosHalfPitch * sinHalfRoll),
		(sinHalfYaw * cosHalfPitch * cosHalfRoll) - (cosHalfYaw * sinHalfPitch * sinHalfRoll),
		(cosHalfYaw * cosHalfPitch * sinHalfRoll) - (sinHalfYaw * sinHalfPitch * cosHalfRoll));
}

Quaternion Quaternion::angleAxis(const float& angle, const Vector3& axis)
{
	float halfAngle = angle * 0.5f;
	return Quaternion(cos(halfAngle), sin(halfAngle) * axis);
}

float Quaternion::magnitude() const
{
	return sqrt((w * w) + v.magnitude2());
}

Quaternion Quaternion::conjugate() const
{
	return Quaternion(w, -v.x, -v.y, -v.z);
}

Quaternion Quaternion::inverse() const
{
	return conjugate() / dot(*this, *this);
}

Matrix4 Quaternion::toMatrix4() const
{
	float x2 = v.x * v.x;
	float y2 = v.y * v.y;
	float z2 = v.z * v.z;
	float wx = w * v.x;
	float wy = w * v.y;
	float wz = w * v.z;
	float xy = v.x * v.y;
	float xz = v.x * v.z;
	float yz = v.y * v.z;
	return Matrix4(
		1.0f - (2.0f * y2) - (2.0f * z2), (2.0f * xy) + (2.0f * wz), (2.0f * xz) - (2.0f * wy), 0.0f,
		(2.0f * xy) - (2.0f * wz), 1.0f - (2.0f * x2) - (2.0f * z2), (2.0f * yz) + (2.0f * wx), 0.0f,
		(2.0f * xz) + (2.0f * wy), (2.0f * yz) - (2.0f * wx), 1.0f - (2.0f * x2) - (2.0f * y2), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);
}

Matrix3 Quaternion::toMatrix3() const
{
	return static_cast<Matrix3>(toMatrix4());
}

Quaternion& Quaternion::operator*=(const Quaternion& rhs)
{
	float rW = (w * rhs.w) - dot(v, rhs.v);
	Vector3 rV = (w * rhs.v) + (rhs.w * v) + cross(v, rhs.v);
	w = rW;
	v = rV;
	return *this;
}

Quaternion& Quaternion::operator*=(const float& rhs)
{
	w *= rhs;
	v *= rhs;
	return *this;
}

Quaternion& Quaternion::operator/=(const float& rhs)
{
	*this *= (1 / rhs);
	return *this;
}

Quaternion& Quaternion::operator-()
{
	w = -w;
	v = -v;
	return *this;
}

Quaternion cliqCity::graphicsMath::normalize(const Quaternion& quaternion)
{
	return Quaternion(quaternion) /= quaternion.magnitude();
}

float cliqCity::graphicsMath::dot(const Quaternion& lhs, const Quaternion& rhs)
{
	return (lhs.w * rhs.w) + dot(lhs.v, rhs.v);
}

Quaternion cliqCity::graphicsMath::operator*(const Quaternion& lhs, const Quaternion& rhs)
{
	return Quaternion(lhs) *= rhs;
}

Quaternion cliqCity::graphicsMath::operator*(const Quaternion& lhs, const float& rhs)
{
	return Quaternion(lhs) *= rhs;
}

Vector3 cliqCity::graphicsMath::operator*(const Vector3& lhs, const Quaternion& rhs)
{
	return rhs.inverse() * lhs;
}

Vector3 cliqCity::graphicsMath::operator*(const Quaternion& lhs, const Vector3& rhs)
{
	Vector3 VxP = cross(lhs.v, rhs);
	Vector3 VxPxV = cross(lhs.v, VxP);
	return rhs + ((VxP * lhs.w) + VxPxV) * 2.0f;
}

Quaternion cliqCity::graphicsMath::operator/(const Quaternion& lhs, const float& rhs)
{
	return lhs * (1.0f / rhs);
}