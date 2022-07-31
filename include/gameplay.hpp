#pragma once
#include <xmath.h>
#include "state.hpp"

namespace nj
{

void step_cell(nj::state& src, nj::state& dst, int r, int c, float dt)
{
    constexpr auto k_drying_rate = 0.99f;
    constexpr auto k_growth_rate = 0.01f;
    constexpr auto k_seed_rate = 0.001f;
    constexpr auto k_burn_rate = 0.5f;
    constexpr auto k_fire_supression = 0.75f;

    auto& cell = src.cells[r][c];

    // if (cell.plants() > 0.0f)
    // {
    // 	std::cerr << "plant: " << cell.plants() << std::endl;
    // }

    const mat<3,3> weights = {
        {0.5, 1.0, 0.5},
        {1.0, 0.0, 1.0},
        {0.5, 1.0, 0.5}
    };

    // stm = np.array([
    //     [ 1, 0, k_growth_rate, -k_burn_rate], # plant density is maintained, and grows as a function of moisture
    //     [ k_seed_rate, 1, 0, 0], # seeds fill as a function of the density of the plants
    //     [ 0, 0, k_drying_rate, -k_burn_rate], # moisture drys out slowly on its own, but can be accelerated by fire
    //     [ 0, 0, -k_fire_supression, 0.85], # fire needs plants to sustain, equal moisture counters fire
    // ])

    mat<4,4> stm = {
        { 1, 0, k_growth_rate, -k_burn_rate}, // plant density is maintained, and grows as a function of moisture
        { k_seed_rate, 1, 0, 0}, // seeds fill as a function of the density of the plants
        { 0, 0, k_drying_rate, -k_burn_rate}, // moisture drys out slowly on its own, but can be accelerated by fire
        { 0, 0, -k_fire_supression, 0.85}, // fire needs plants to sustain, equal moisture counters fire
    };

    stm *= dt;

    auto& dst_cell = dst.cells[r][c];
    dst_cell = stm * cell;
    dst_cell.fire(dst_cell.plants() * dst_cell.fire());

    for (unsigned i = 4; i--;)
    {
    	dst_cell[i] = std::max<float>(std::min<float>(dst_cell[i], 1), 0);
    }
}

void step_world(nj::state& state, float dt)
{
	auto update_stride = state.width() / 2;
	auto updates = state.active_cells.size() / update_stride;

	for (unsigned i = updates; i--;)
	{
		auto j = (state.frame + i * update_stride) % state.active_cells.size();
		auto& coord = state.active_cells[j];

		step_cell(state, state, coord[0], coord[1], dt);
	} 
}

}