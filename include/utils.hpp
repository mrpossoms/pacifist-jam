#pragma once
#include <xmath.h>

namespace nj
{

template<size_t N>
vec<N> random_norm_vec(std::default_random_engine& generator, float mean=0, float stddev=.33)
{
	std::normal_distribution<float> n(mean, stddev);
	vec<N> out;

	for (unsigned i = 0; i < N; i++) { out[i] = n(generator); }

	return out;
}

}