#pragma once
#include "Grid.h"
#include <Rig3D/Common/Transform.h>
#include <RayCast.h>
#include <Rig3D/Common/Timer.h>
#include <functional>
#include "Colors.h"
#include "trace.h"

#define RAD_TO_DEG 57.29578f
#define DEG_TO_RAD 0.01745329f
#define MOD(a, n) ((a) - (floorf((a) / (n)) * (n)))

//static Vector3 gRepelOffset = { 0.66f, -0.6f, 0.0f };
static Vector3 gRepelOffset = { 1.4f, -1.4f, 0.0f };

static float gRepelFocus = 6.0f;
static float gRepelCastDistance = 4.0f;
static float gRepelIncrement = 0.2f;
static float gRapelDecay = 0.1f;
static float gMaxRepel = 1.0f;

static float gTurnRate = 50;
static float gMoveSpeed = 20;

using namespace PathFinder;

namespace Rig3D
{
	
	class TargetFollower
	{
	public:
		Grid&				mGrid;
		Transform&			mTransform;
		SearchResult<Node>	mSearchResult;
		AABB<vec2f>*		mAABBs;
		int					mAABBCount;

		float				mReverseTime = 0;
		float				mRepel = 0;

		TargetFollower(Transform& transform, AABB<vec2f>* aabbs, int aabbCount);
		~TargetFollower();

		void MoveTowards(Transform& target)
		{
			auto targetPosition = target.GetPosition();
			auto position = mTransform.GetPosition();
			auto deltaTime = 0.1f;//Timer::SharedInstance().GetDeltaTime();

			Ray<vec2f> ray;
			RayCastHit<vec2f> hit;

			mSearchResult = mGrid.GetPath(position, targetPosition);
			if (mSearchResult.path.size() > 1)
			{
				for (auto node = mSearchResult.path.rbegin(); node != mSearchResult.path.rend(); ++node)
				{
					auto distance = (*node)->position - position;
					auto mag = magnitude(distance);
					auto direction = distance * (1 / mag);

					ray = { position, direction };
					if (!RayCast(&hit, ray, mAABBs, mAABBCount, mag))
					{
						auto test = node;
						if (++test == mSearchResult.path.rend())
						{
							--node;
						}

						targetPosition = (*node)->position;

						TRACE_LINE(position, targetPosition, Colors::cyan);
						break;
					}
				}
			}

			auto currentAngle = mTransform.GetRollPitchYaw().z * RAD_TO_DEG;
				
			auto targetDistance = targetPosition - position;
			auto targetAngle = atan2f(-targetDistance.x, targetDistance.y) * RAD_TO_DEG;
			
			auto da = abs(MOD(targetAngle - currentAngle + 180.0f, 360.0f) - 180);

			auto targetRotation = Quaternion::rollPitchYaw(targetAngle * DEG_TO_RAD, 0, 0);

			Quaternion rotation;
			RotateTowards(&rotation, mTransform.GetRotation(), targetRotation, gTurnRate * 5 * deltaTime);
			mTransform.SetRotation(rotation);

			auto frontOffset = mTransform.TransformPoint(vec3f(0, 1, 0) * gRepelFocus);
			auto rightOffset = mTransform.TransformPoint(gRepelOffset);
			auto leftOffset  = mTransform.TransformPoint(vec3f(-gRepelOffset.x, gRepelOffset.y, 0));

			TRACE_LINE(rightOffset, rightOffset +  normalize(frontOffset - rightOffset) * gRepelCastDistance, Colors::blue);
			ray = { rightOffset, normalize(frontOffset - rightOffset) };
			if (RayCast(&hit, ray, mAABBs, mAABBCount, gRepelCastDistance)) // i need cast distance here (repel cast distance)
			{
				mRepel = fmax(mRepel - gRepelIncrement, -gMaxRepel);
			}

			TRACE_LINE(leftOffset, leftOffset + normalize(frontOffset - leftOffset) * gRepelCastDistance, Colors::blue);
			ray = { leftOffset, normalize(frontOffset - leftOffset) };
			if (RayCast(&hit, ray, mAABBs, mAABBCount, gRepelCastDistance)) // i need cast distance here (repel cast distance)
			{
				mRepel = fmin(mRepel + gRepelIncrement, +gMaxRepel);
			}

			mRepel = mRepel * (1 - gRapelDecay); // Mathf.Abs(repel) < .01f ? 0 : repel * .8f;


			if (da > 90)
			{
				mReverseTime = .25f;
			}

			auto reverse = false;
			if (mReverseTime > 0)
			{
				reverse = true;
				mReverseTime -= deltaTime;
				//return;
			}

			//var reverse = da > 60;
			//if (reverse)
			//    return;

			auto moveDirection = Vector3(mRepel, (1 - abs(mRepel)) * (reverse ? -1 : 1), 0);

			// "forward"
			//Debug.DrawLine(transform.position, transform.position + transform.up);
			TRACE_LINE(mTransform.GetPosition(), mTransform.GetPosition() + mTransform.GetUp() * 3 /*+ transform.TransformDirection(moveDirection).normalized*/, Colors::red);

			/*if (GetComponent<Animator>())
			{
				GetComponent<Animator>().SetFloat("Slide", repel);
			}*/

			// scale foword by move direction
			auto r2 = normalize(mTransform.GetRotation());
			auto moveStep = r2 * moveDirection;
			/*moveStep.x *= moveDirection.x;
			moveStep.y *= moveDirection.y;
			moveStep.z *= moveDirection.z;
*/
			//TRACE(float(deltaTime));
			mTransform.SetPosition(position + moveStep * gMoveSpeed * .1f * deltaTime);
		}

		static void RotateTowards(Quaternion* out, const Quaternion& from, const Quaternion& to, float maxDegreesDelta)
		{
			float num = angleBetween(from, to);
			if (double(num) == 0.0)
			{
				*out = to;
				return;
			}
			float t = fmin(1.0f, maxDegreesDelta / num);
			return Slerp(out, from, to, t);
		}

		static void Slerp(Quaternion* out, const Quaternion& from, const Quaternion& to, const float u)
		{
			Quaternion q0(from);
			Quaternion q1(to);

			float cosAngle = dot(q0, q1);
			if (cosAngle < 0.0f) {
				q1 = -q1;
				cosAngle = -cosAngle;
			}

			float k0, k1;				// Check for divide by zero
			if (cosAngle > 0.9999f) {
				k0 = 1.0f - u;
				k1 = u;
			}
			else {
				float angle = acosf(cosAngle);
				float oneOverSinAngle = 1.0f / sinf(angle);

				k0 = ((sinf(1.0f - u) * angle) * oneOverSinAngle);
				k1 = (sinf(u * angle) * oneOverSinAngle);
			}

			q0 = q0 * k0;
			q1 = q1 * k1;

			out->w = q0.w + q1.w;
			out->v.x = q0.v.x + q1.v.x;
			out->v.y = q0.v.y + q1.v.y;
			out->v.z = q0.v.z + q1.v.z;
		}
		
		// TODO! Move angleBetween to GraphicsMath Quaternion.cpp
		#define M_RAD_TO_DEG 57.2957801818848f

		// Returns the angle in degrees between two rotations /a/ and /b/.
		static float angleBetween(const Quaternion& lhs, const Quaternion& rhs)
		{
			return float(double(acos(fmin(abs(cliqCity::graphicsMath::dot(lhs, rhs)), 1.0f))) * 2.0f * M_RAD_TO_DEG);
		}
	};

}
