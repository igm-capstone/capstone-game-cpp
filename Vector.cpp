#define _USE_MATH_DEFINES
#include <cmath>
#include "Vector.hpp"

#define SIMD 1

using namespace cliqCity::graphicsMath;

// Vector2

// Compound Assignment (Vector2)

Vector2& Vector2::operator+=(const Vector2& rhs)
{
	this->x += rhs.x;
	this->y += rhs.y;
	return *this;
}

Vector2& Vector2::operator-=(const Vector2& rhs)
{
	this->x -= rhs.x;
	this->y -= rhs.y;
	return *this;
}

Vector2& Vector2::operator*=(const Vector2& rhs)
{
	this->x *= rhs.x;
	this->y *= rhs.y;
	return *this;
}

// Compound Assignment (float)

Vector2& Vector2::operator+=(const float& rhs)
{
	this->x += rhs;
	this->y += rhs;
	return *this;
}

Vector2& Vector2::operator-=(const float& rhs)
{
	this->x -= rhs;
	this->y -= rhs;
	return *this;
}

Vector2& Vector2::operator*=(const float& rhs)
{
	this->x *= rhs;
	this->y *= rhs;
	return *this;
}

Vector2& Vector2::operator/=(const float& rhs)
{
	return (*this *= (1.0f / rhs));
}

// Unary

Vector2& Vector2::operator++()
{
	return (*this += 1.0f);
}

Vector2& Vector2::operator--()
{
	return (*this -= 1.0f);
}

Vector2& Vector2::operator=(const Vector2& rhs)
{
	x = rhs.x;
	y = rhs.y;
	return *this;
}

Vector2& Vector2::operator-()
{
	x = -x;
	y = -y;
	return *this;
}

float& Vector2::operator[](const unsigned int& index)
{
	return reinterpret_cast<float *>(this)[index];
}

// Vector3

// Compound Assignment (Vector3)

Vector3& Vector3::operator+=(const Vector3& rhs)
{
	this->x += rhs.x;
	this->y += rhs.y;
	this->z += rhs.z;
	return *this;
}

Vector3& Vector3::operator-=(const Vector3& rhs)
{
	this->x -= rhs.x;
	this->y -= rhs.y;
	this->z -= rhs.z;
	return *this;
}

Vector3& Vector3::operator*=(const Vector3& rhs)
{
	this->x *= rhs.x;
	this->y *= rhs.y;
	this->z *= rhs.z;
	return *this;
}

// Compound Assignment (float)

Vector3& Vector3::operator+=(const float& rhs)
{
	this->x += rhs;
	this->y += rhs;
	this->z += rhs;
	return *this;
}

Vector3& Vector3::operator-=(const float& rhs)
{
	this->x -= rhs;
	this->y -= rhs;
	this->z -= rhs;
	return *this;
}

Vector3& Vector3::operator*=(const float& rhs)
{
	this->x *= rhs;
	this->y *= rhs;
	this->z *= rhs;
	return *this;
}

Vector3& Vector3::operator/=(const float& rhs)
{
	return (*this *= (1.0f / rhs));
}

// Unary

Vector3& Vector3::operator++()
{
	return (*this += 1.0f);
}

Vector3& Vector3::operator--()
{
	return (*this -= 1.0f);
}

Vector3& Vector3::operator=(const Vector3& rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	return *this;
}

Vector3& Vector3::operator-()
{
	x = -x;
	y = -y;
	z = -z;
	return *this;
}

float& Vector3::operator[](const unsigned int& index)
{
	return reinterpret_cast<float*>(this)[index];
}

Vector3::operator Vector2()
{
	return Vector2(this->x, this->y);
}

// Vector4

// Compound Assignment (Vector4)

Vector4& Vector4::operator+=(const Vector4& rhs)
{
#ifdef SIMD
	this->v = simd::Add(this->v, rhs.v);
#else
	this->x += rhs.x;
	this->y += rhs.y;
	this->z += rhs.z;
	this->w += rhs.w;
#endif
	return *this;
}

Vector4& Vector4::operator-=(const Vector4& rhs)
{
#ifdef SIMD
	this->v = simd::Subtract(this->v, rhs.v);
#else
	this->x -= rhs.x;
	this->y -= rhs.y;
	this->z -= rhs.z;
	this->w -= rhs.w;
#endif
	return *this;
}

Vector4& Vector4::operator*=(const Vector4& rhs)
{
#ifdef SIMD
	this->v = simd::Multiply(this->v, rhs);
#else
	this->x *= rhs.x;
	this->y *= rhs.y;
	this->z *= rhs.z;
	this->w *= rhs.w;
#endif
	return *this;
}

// Compound Assignment (float)

Vector4& Vector4::operator+=(const float& rhs)
{
#ifdef SIMD
	this->v = simd::Add(this->v, simd::Set(rhs));
#else
	this->x += rhs;
	this->y += rhs;
	this->z += rhs;
	this->w += rhs;
#endif
	return *this;
}

Vector4& Vector4::operator-=(const float& rhs)
{
#ifdef SIMD
	this->v = simd::Subtract(this->v, simd::Set(rhs));
#else
	this->x -= rhs;
	this->y -= rhs;
	this->z -= rhs;
	this->w -= rhs;
#endif
	return *this;
}

Vector4& Vector4::operator*=(const float& rhs)
{
#ifdef SIMD
	this->v = simd::Multiply(this->v, simd::Set(rhs));
#else
	this->x *= rhs;
	this->y *= rhs;
	this->z *= rhs;
	this->w *= rhs;
#endif
	return *this;
}

Vector4& Vector4::operator/=(const float& rhs)
{
	return (*this *= (1.0f / rhs));
}

// Unary

Vector4& Vector4::operator++()
{
	return (*this += 1.0f);
}

Vector4& Vector4::operator--()
{
	return (*this -= 1.0f);
}

Vector4& Vector4::operator=(const Vector4& rhs)
{
#ifdef SIMD
	this->v = rhs.v;
#else
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	w = rhs.w;
#endif
	return *this;
}

Vector4& Vector4::operator-()
{
#ifdef SIMD
	this->v = simd::Negate(this->v);
#else
	x = -x;
	y = -y;
	z = -z;
	w = -w;
#endif
	return *this;
}

float& Vector4::operator[](const unsigned int& index)
{
	return reinterpret_cast<float *>(this)[index];
}

// Dot, Cross, Normalize

Vector3 cliqCity::graphicsMath::cross(const Vector3& lhs, const Vector3& rhs)
{
	return {
		(lhs.y * rhs.z) - (lhs.z * rhs.y),
		(lhs.z * rhs.x) - (lhs.x * rhs.z),
		(lhs.x * rhs.y) - (lhs.y * rhs.x) };
}

float cliqCity::graphicsMath::magnitudeSquared(const Vector2& vector)
{
	return dot(vector, vector);
}

float cliqCity::graphicsMath::magnitudeSquared(const Vector3& vector)
{
	return dot(vector, vector);
}

float cliqCity::graphicsMath::magnitudeSquared(const Vector4& vector)
{
#ifdef SIMD
	return simd::MagnitudeSquared(vector.v);
#else
	return dot(vector, vector);
#endif
}

float cliqCity::graphicsMath::magnitude(const Vector2& vector)
{
	return sqrtf(dot(vector, vector));
}

float cliqCity::graphicsMath::magnitude(const Vector3& vector)
{
	return sqrtf(dot(vector, vector));
}

float cliqCity::graphicsMath::magnitude(const Vector4& vector)
{
#ifdef SIMD
	return simd::Magnitude(vector.v);
#else
	return sqrtf(dot(vector, vector));
#endif
}

Vector2 cliqCity::graphicsMath::normalize(const Vector2& vector)
{
	return vector / magnitude(vector);
}

Vector3 cliqCity::graphicsMath::normalize(const Vector3& vector)
{
	return vector / magnitude(vector);
}

Vector4 cliqCity::graphicsMath::normalize(const Vector4& vector)
{
#ifdef SIMD
	return simd::Normalize(vector.v);
#else
	return vector / magnitude(vector);
#endif
}

float cliqCity::graphicsMath::dot(const Vector2& lhs, const Vector2& rhs)
{
	return (lhs.x * rhs.x) + (lhs.y * rhs.y);
}

float cliqCity::graphicsMath::dot(const Vector3& lhs, const Vector3& rhs)
{
	return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
}

float cliqCity::graphicsMath::dot(const Vector4& lhs, const Vector4& rhs)
{
#ifdef SIMD
	return simd::Dot(lhs.v, rhs.v);
#else
	return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z) + (lhs.w * rhs.w);
#endif
}

// Binary (Vector2)

Vector2 cliqCity::graphicsMath::operator+(const Vector2& lhs, const Vector2& rhs)
{
	return Vector2(lhs) += rhs;
}

Vector2 cliqCity::graphicsMath::operator-(const Vector2& lhs, const Vector2& rhs)
{
	return Vector2(lhs) -= rhs;
}

Vector2 cliqCity::graphicsMath::operator*(const Vector2& lhs, const Vector2& rhs)
{
	return Vector2(lhs) *= rhs;
}

Vector2 cliqCity::graphicsMath::operator+(const Vector2& lhs, const float& rhs)
{
	return Vector2(lhs) += rhs;
}

Vector2 cliqCity::graphicsMath::operator-(const Vector2& lhs, const float& rhs)
{
	return Vector2(lhs) -= rhs;
}

Vector2 cliqCity::graphicsMath::operator*(const Vector2& lhs, const float& rhs)
{
	return Vector2(lhs) *= rhs;
}

Vector2 cliqCity::graphicsMath::operator/(const Vector2& lhs, const float& rhs)
{
	return Vector2(lhs) /= rhs;
}

Vector2 cliqCity::graphicsMath::operator+(const float& lhs, const Vector2& rhs)
{
	return rhs + lhs;
}

Vector2 cliqCity::graphicsMath::operator-(const float& lhs, const Vector2& rhs)
{
	return rhs - lhs;
}

Vector2 cliqCity::graphicsMath::operator*(const float& lhs, const Vector2& rhs)
{
	return rhs * lhs;
}

// Binary (Vector3)

Vector3 cliqCity::graphicsMath::operator+(const Vector3& lhs, const Vector3& rhs)
{
	return Vector3(lhs) += rhs;
}

Vector3 cliqCity::graphicsMath::operator-(const Vector3& lhs, const Vector3& rhs)
{
	return Vector3(lhs) -= rhs;
}

Vector3 cliqCity::graphicsMath::operator*(const Vector3& lhs, const Vector3& rhs)
{
	return Vector3(lhs) *= rhs;
}

Vector3 cliqCity::graphicsMath::operator+(const Vector3& lhs, const float& rhs)
{
	return Vector3(lhs) += rhs;
}

Vector3 cliqCity::graphicsMath::operator-(const Vector3& lhs, const float& rhs)
{
	return Vector3(lhs) -= rhs;
}

Vector3 cliqCity::graphicsMath::operator*(const Vector3& lhs, const float& rhs)
{
	return Vector3(lhs) *= rhs;
}

Vector3 cliqCity::graphicsMath::operator/(const Vector3& lhs, const float& rhs)
{
	return Vector3(lhs) /= rhs;
}

Vector3 cliqCity::graphicsMath::operator+(const float& lhs, const Vector3& rhs)
{
	return rhs + lhs;
}

Vector3 cliqCity::graphicsMath::operator-(const float& lhs, const Vector3& rhs)
{
	return rhs - lhs;
}

Vector3 cliqCity::graphicsMath::operator*(const float& lhs, const Vector3& rhs)
{
	return rhs * lhs;
}

// Binary (Vector4)

Vector4 cliqCity::graphicsMath::operator+(const Vector4& lhs, const Vector4& rhs)
{
	return Vector4(lhs) += rhs;
}

Vector4 cliqCity::graphicsMath::operator-(const Vector4& lhs, const Vector4& rhs)
{
	return Vector4(lhs) -= rhs;
}

Vector4 cliqCity::graphicsMath::operator*(const Vector4& lhs, const Vector4& rhs)
{
	return Vector4(lhs) *= rhs;
}

Vector4 cliqCity::graphicsMath::operator+(const Vector4& lhs, const float& rhs)
{
	return Vector4(lhs) += rhs;
}

Vector4 cliqCity::graphicsMath::operator-(const Vector4& lhs, const float& rhs)
{
	return Vector4(lhs) -= rhs;
}

Vector4 cliqCity::graphicsMath::operator*(const Vector4& lhs, const float& rhs)
{
	return Vector4(lhs) *= rhs;
}

Vector4 cliqCity::graphicsMath::operator/(const Vector4& lhs, const float& rhs)
{
	return Vector4(lhs) /= rhs;
}

Vector4 cliqCity::graphicsMath::operator+(const float& lhs, const Vector4& rhs)
{
	return rhs + lhs;
}

Vector4 cliqCity::graphicsMath::operator-(const float& lhs, const Vector4& rhs)
{
	return rhs - lhs;
}

Vector4 cliqCity::graphicsMath::operator*(const float& lhs, const Vector4& rhs)
{
	return rhs * lhs;
}