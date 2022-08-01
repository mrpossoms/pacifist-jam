#pragma once
#include <g.h>
#include <xmath.h>
#include "state.hpp"

namespace nj
{

void step_cell(nj::state& src, nj::state& dst, int r, int c, float dt)
{
    const float k_drying_rate = 0.99f;
    const float k_growth_rate = 0.01f;
    const float k_seed_rate = 0.0001f;
    const float k_burn_rate = 0.95f;
    const float k_fire_supression = 0.75f;

    std::uniform_real_distribution<float> dice_roll(0.0, 1.0);

    auto& cell = src.cells[r][c];

    // if (cell.plants() > 0.0f)
    // {
    // 	std::cerr << "plant: " << cell.plants() << std::endl;
    // }

    const mat<3,3, float> weights = {
        {0.5f, 1.0f, 0.5f},
        {1.0f, 0.0f, 1.0f},
        {0.5f, 1.0f, 0.5f}
    };

    // stm = np.array([
    //     [ 1, 0, k_growth_rate, -k_burn_rate], # plant density is maintained, and grows as a function of moisture
    //     [ k_seed_rate, 1, 0, 0], # seeds fill as a function of the density of the plants
    //     [ 0, 0, k_drying_rate, -k_burn_rate], # moisture drys out slowly on its own, but can be accelerated by fire
    //     [ 0, 0, -k_fire_supression, 0.85], # fire needs plants to sustain, equal moisture counters fire
    // ])

    mat<4,4, float> stm = {
        { 1, 0, k_growth_rate, -k_burn_rate}, // plant density is maintained, and grows as a function of moisture
        { k_seed_rate, 1, 0, 0}, // seeds fill as a function of the density of the plants
        { 0, 0, k_drying_rate, -k_burn_rate}, // moisture drys out slowly on its own, but can be accelerated by fire
        { 0, 0, -k_fire_supression, 0.85}, // fire needs plants to sustain, equal moisture counters fire
    };

    stm *= dt;

    auto& dst_cell = dst.cells[r][c];
    dst_cell = stm * cell;
    dst_cell.fire(dst_cell.plants() * dst_cell.fire());
    dst_cell.seed_timer += dst_cell.seed();

    for (int ri = -1; ri <= 1; ri++)
    for (int ci = -1; ci <= 1; ci++)
    {
        auto rr = std::min<int>(std::max<int>(0, ri + r), src.depth()-1);
        auto cc = std::min<int>(std::max<int>(0, ci + c), src.width()-1);

        auto neighbor_fire = src.cells[rr][cc].fire();

        auto next_fire = cell.fire() + cell.plants() * neighbor_fire * k_burn_rate * weights[ri + 1][ci + 1];

        //if (neighbor_fire > 0)
        //{
        //    std::cerr << "fire: " << next_fire << std::endl;
        //}

        dst_cell.fire(next_fire);
        dst_cell.plants(cell.plants() + src.cells[rr][cc][0] * k_growth_rate * weights[ri + 1][ci + 1]);
    }

    if (dst_cell.seed_timer > dice_roll(src.rng))
    {
        auto pos = vec<3>{ (float)r, cell.elevation, (float)c } + (nj::random_norm_vec<3>(src.rng, 0.f, 0.33f) * vec<3>{0.5, 4 * cell.plants(), 0.5}) + vec<3>{0, 1, 0};
        dst.seed_positions.push_back(pos);
        dst.seed_velocities.push_back({});
        dst_cell.seed_timer = 0;
    }

    for (unsigned i = 4; i--;)
    {
    	dst_cell[i] = std::max<float>(std::min<float>(dst_cell[i], 1), 0);
        dst_cell[i] = dst_cell[i] < 0.001 ? 0 : dst_cell[i];
    }

    dst.forrest_density += dst_cell.plants();
}

void handle_seeds(nj::state& state, float dt)
{
    auto cam_speed = state.camera.velocity.magnitude();
    auto tip = state.camera.position + state.camera.forward() * 2;
    auto capture_rad = pow(4, 2);
    for (unsigned i = 0; i < state.seed_positions.size(); i++)
    {
        auto& seed_pos = state.seed_positions[i];
        auto& seed_vel = state.seed_velocities[i];
        auto d = tip - seed_pos;

        auto seed_speed = seed_vel.magnitude();

        if (d.dot(d) <= capture_rad && glfwGetMouseButton(g::gfx::GLFW_WIN, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            //seed_vel = state.camera.velocity;
            auto error = tip - seed_pos;

            seed_vel += error * 0.75;

        }

        if (seed_speed > 0.1)
        {
            seed_vel += vec<3>{0, -0.98, 0} * dt;
            seed_vel += nj::random_norm_vec<3>(state.rng) * 40 * dt;            
        }

        seed_pos += seed_vel * dt;
    
        if (seed_pos[1] <= state.terrain_hm.elevation_at(seed_pos))
        {
            state.seed_positions.remove_at(i);
            state.seed_velocities.remove_at(i);
            auto r = std::min<int>(state.depth() - 1, std::max(0, (int)seed_pos[0]));
            auto c = std::min<int>(state.width() - 1, std::max(0, (int)seed_pos[2]));
            auto& landing_cell = state.cells[r][c];
            landing_cell.plants(landing_cell.plants() + 0.2);
        }
    }
}

void step_world(nj::state& state, float dt)
{
    std::uniform_real_distribution<float> dice_roll(0.0, 1.0);
	auto update_stride = state.width() / 2;
	auto updates = state.active_cells.size() / update_stride;

    auto d_0 = state.forrest_density;

	for (unsigned i = updates; i--;)
	{
		auto j = (state.frame + i * update_stride) % state.active_cells.size();
		auto& coord = state.active_cells[j];

		step_cell(state, state, coord[0], coord[1], dt);
	}

    handle_seeds(state, dt);

    state.forrest_density = 0;
    for (auto& coord : state.active_cells)
    {
        state.forrest_density += state.cells[coord[0]][coord[1]].plants();
    }

    if (dice_roll(state.rng) < state.forrest_density / (float)state.active_cells.size())
    {
        std::uniform_int_distribution<int> cell_picker(0, state.active_cells.size() - 1);

        auto& coord = state.active_cells[cell_picker(state.rng)];
        state.cells[coord[0]][coord[1]].fire(1); // dice_roll(state.rng));
    }

    std::cerr << state.forrest_density << std::endl;
}

}