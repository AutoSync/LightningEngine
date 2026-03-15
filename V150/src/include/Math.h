// Math.h — Lightning Engine math utilities.
// Free functions in the LMath namespace. No external dependencies.
// Operates on Lightning::V2 / V3 and plain floats.

#pragma once
#include <cmath>
#include "Types.h"

namespace LMath
{
	// -----------------------------------------------------------------------
	// Scalar
	// -----------------------------------------------------------------------

	inline float Lerp(float a, float b, float t)
	{
		return a + (b - a) * t;
	}

	inline float Clamp(float v, float min, float max)
	{
		if (v < min) return min;
		if (v > max) return max;
		return v;
	}

	inline float Clamp01(float v) { return Clamp(v, 0.f, 1.f); }

	inline float Abs(float v)  { return v < 0.f ? -v : v; }
	inline float Sign(float v) { return v < 0.f ? -1.f : (v > 0.f ? 1.f : 0.f); }
	inline float Min(float a, float b) { return a < b ? a : b; }
	inline float Max(float a, float b) { return a > b ? a : b; }

	// Smooth Hermite interpolation: 0 at edges, smooth in between.
	inline float Smoothstep(float edge0, float edge1, float t)
	{
		t = Clamp01((t - edge0) / (edge1 - edge0));
		return t * t * (3.f - 2.f * t);
	}

	// Map a value from [inMin, inMax] to [outMin, outMax].
	inline float Remap(float v, float inMin, float inMax, float outMin, float outMax)
	{
		return outMin + (v - inMin) * (outMax - outMin) / (inMax - inMin);
	}

	inline float MoveTowards(float current, float target, float maxDelta)
	{
		float diff = target - current;
		if (Abs(diff) <= maxDelta) return target;
		return current + Sign(diff) * maxDelta;
	}

	// -----------------------------------------------------------------------
	// V2
	// -----------------------------------------------------------------------

	inline float Dot(Lightning::V2 a, Lightning::V2 b)
	{
		return a.x * b.x + a.y * b.y;
	}

	inline float LengthSq(Lightning::V2 v)
	{
		return v.x * v.x + v.y * v.y;
	}

	inline float Length(Lightning::V2 v)
	{
		return std::sqrt(LengthSq(v));
	}

	inline float Distance(Lightning::V2 a, Lightning::V2 b)
	{
		return Length(a - b);
	}

	inline float DistanceSq(Lightning::V2 a, Lightning::V2 b)
	{
		return LengthSq(a - b);
	}

	// Returns zero vector if already zero.
	inline Lightning::V2 Normalize(Lightning::V2 v)
	{
		float len = Length(v);
		if (len < 1e-8f) return { 0.f, 0.f };
		return v / len;
	}

	inline Lightning::V2 Lerp(Lightning::V2 a, Lightning::V2 b, float t)
	{
		return { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t) };
	}

	inline Lightning::V2 Clamp(Lightning::V2 v, Lightning::V2 min, Lightning::V2 max)
	{
		return { Clamp(v.x, min.x, max.x), Clamp(v.y, min.y, max.y) };
	}

	inline Lightning::V2 MoveTowards(Lightning::V2 current, Lightning::V2 target, float maxDelta)
	{
		Lightning::V2 diff = target - current;
		float dist = Length(diff);
		if (dist <= maxDelta || dist < 1e-8f) return target;
		return current + diff / dist * maxDelta;
	}

	// Angle in radians between two V2 directions.
	inline float Angle(Lightning::V2 a, Lightning::V2 b)
	{
		return std::acos(Clamp01(Dot(Normalize(a), Normalize(b))));
	}

	// Angle of a vector relative to positive X axis (radians, -PI..PI).
	inline float Atan2(Lightning::V2 v)
	{
		return std::atan2(v.y, v.x);
	}

	// Reflect v around a normal n (n must be normalised).
	inline Lightning::V2 Reflect(Lightning::V2 v, Lightning::V2 n)
	{
		return v - n * (2.f * Dot(v, n));
	}

	// Perpendicular (rotated 90° CCW).
	inline Lightning::V2 Perp(Lightning::V2 v)
	{
		return { -v.y, v.x };
	}

	// -----------------------------------------------------------------------
	// V3
	// -----------------------------------------------------------------------

	inline float Dot(Lightning::V3 a, Lightning::V3 b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	inline float LengthSq(Lightning::V3 v)
	{
		return v.x * v.x + v.y * v.y + v.z * v.z;
	}

	inline float Length(Lightning::V3 v)
	{
		return std::sqrt(LengthSq(v));
	}

	inline Lightning::V3 Normalize(Lightning::V3 v)
	{
		float len = Length(v);
		if (len < 1e-8f) return { 0.f, 0.f, 0.f };
		return { v.x / len, v.y / len, v.z / len };
	}

	inline Lightning::V3 Cross(Lightning::V3 a, Lightning::V3 b)
	{
		return {
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		};
	}

	inline Lightning::V3 Lerp(Lightning::V3 a, Lightning::V3 b, float t)
	{
		return { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t) };
	}

} // namespace LMath
