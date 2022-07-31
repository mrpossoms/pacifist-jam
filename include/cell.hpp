#pragma once
#include <xmath.h>
#include <optional>
using namespace xmath;

namespace nj
{

struct cell : public vec<4>
{
	float elevation;
	float seed;

	inline float plants(std::optional<float> v = std::nullopt)
	{ 
		if (v) { (*this)[0] = v.value(); }
		return (*this)[0];
	}

	inline float seed(std::optional<float> v = std::nullopt)
	{ 
		if (v) { (*this)[1] = v.value(); }
		return (*this)[1];
	}

	inline float moisture(std::optional<float> v = std::nullopt)
	{ 
		if (v) { (*this)[2] = v.value(); }
		return (*this)[2];
	}

	inline float fire(std::optional<float> v = std::nullopt)
	{ 
		if (v) { (*this)[3] = v.value(); }
		return (*this)[3];
	}

	void operator=(const vec<4>& v)
	{
		for (unsigned i = 4; i--;) { this->v[i] = v[i]; }
	}

	inline bool is_active() const { return elevation >= 4; }
};

} // namespace nj