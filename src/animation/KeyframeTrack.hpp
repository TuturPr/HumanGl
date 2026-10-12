#pragma once

#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

// A looping curve through key values. Between two keys the value follows a
// cubic Hermite spline whose tangents are those of a Catmull-Rom spline, so
// position AND velocity are continuous everywhere, including across the loop.
// T only needs `T + T` and `T * float` (float, LegPose, Pose ...).
template <typename T>
class KeyframeTrack
{
public:
	struct Key
	{
		float phase;	// in [0, 1), strictly increasing
		T value;
	};

	explicit KeyframeTrack(std::vector<Key> keys) : keys_(std::move(keys))
	{
		if (keys_.size() < 2)
			throw std::invalid_argument("KeyframeTrack needs at least two keys");
		for (std::size_t i = 0; i < keys_.size(); ++i)
		{
			const bool inRange = keys_[i].phase >= 0.0f && keys_[i].phase < 1.0f;
			const bool increasing = i == 0 || keys_[i].phase > keys_[i - 1].phase;
			if (!inRange || !increasing)
				throw std::invalid_argument("KeyframeTrack phases must increase within [0, 1)");
		}
	}

	// `phase` is wrapped into [0, 1): one unit of phase is one loop.
	T sample(float phase) const
	{
		phase -= std::floor(phase);
		const int n = static_cast<int>(keys_.size());

		int i1 = n - 1;	// last key at or before `phase` (wraps to the last key)
		for (int i = 0; i < n; ++i)
			if (keys_[i].phase <= phase)
				i1 = i;

		const float t0 = phaseOf(i1 - 1);
		const float t1 = phaseOf(i1);
		const float t2 = phaseOf(i1 + 1);
		const float t3 = phaseOf(i1 + 2);
		const T& p0 = valueOf(i1 - 1);
		const T& p1 = valueOf(i1);
		const T& p2 = valueOf(i1 + 1);
		const T& p3 = valueOf(i1 + 2);

		float local = phase;
		if (local < t1)
			local += 1.0f;	// before the first key: we are in the wrapping segment
		const float span = t2 - t1;
		const float u = (local - t1) / span;

		// Hermite basis.
		const float u2 = u * u;
		const float u3 = u2 * u;
		const float h00 = 2.0f * u3 - 3.0f * u2 + 1.0f;
		const float h10 = u3 - 2.0f * u2 + u;
		const float h01 = -2.0f * u3 + 3.0f * u2;
		const float h11 = u3 - u2;

		// Catmull-Rom tangents m1 = (p2 - p0) / (t2 - t0), m2 = (p3 - p1) / (t3 - t1),
		// expanded so that the result is a weighted sum of the four keys:
		// p = h00 p1 + h01 p2 + h10 span m1 + h11 span m2
		const float k1 = h10 * span / (t2 - t0);
		const float k2 = h11 * span / (t3 - t1);
		return p0 * (-k1) + p1 * (h00 - k2) + p2 * (h01 + k1) + p3 * k2;
	}

private:
	// Phase of key `i` on an unwrapped time line (key n is key 0 one loop later).
	float phaseOf(int i) const
	{
		const int n = static_cast<int>(keys_.size());
		const int loops = (i < 0) ? -1 : i / n;
		return keys_[static_cast<std::size_t>(i - loops * n)].phase + static_cast<float>(loops);
	}

	const T& valueOf(int i) const
	{
		const int n = static_cast<int>(keys_.size());
		return keys_[static_cast<std::size_t>(((i % n) + n) % n)].value;
	}

	std::vector<Key> keys_;
};
