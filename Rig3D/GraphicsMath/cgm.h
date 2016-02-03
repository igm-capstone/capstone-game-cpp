#pragma once
#include "Vector.hpp"
#include "Matrix.hpp"
#include "Quaternion.hpp"
#include "Operations.hpp"
#include "SIMDVector.hpp"
#include "SIMDMatrix.hpp"

#define PI 3.14159265359f

typedef cliqCity::graphicsMath::SIMDMatrix		fast_mat;
typedef cliqCity::graphicsMath::SIMDVector		fast_vec;
typedef cliqCity::graphicsMath::Matrix4			mat4f;
typedef cliqCity::graphicsMath::Matrix3			mat3f;
typedef cliqCity::graphicsMath::Vector4			vec4f;
typedef cliqCity::graphicsMath::Vector3			vec3f;
typedef cliqCity::graphicsMath::Vector2			vec2f;
typedef cliqCity::graphicsMath::Quaternion		quatf;

// Left Handed Coordinate System
// Row Major Vector / Matrix Operations