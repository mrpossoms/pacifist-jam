#pragma once
#include <g.h>
#include <xmath.h>
#include <cell.hpp>
#include <utils.hpp>
#include <camera.hpp>

using namespace xmath;

namespace nj
{

struct state
{
	int plant_block_size = 225;
	std::vector<std::vector<nj::cell>> cells;
	std::vector<vec<2, unsigned>> active_cells;
	g::bounded_list<vec<3>, 1024> seed_positions;
	g::bounded_list<vec<3>, 1024> seed_velocities;

	nj::flying_cam camera;
	g::game::sdf terrain;
	std::default_random_engine rng;
	g::game::heightmap terrain_hm;

	float forrest_density = 0;


	float t = 0;
	unsigned frame = 0;

	state(const size_t w, const size_t d) : 
		terrain_hm([&](const vec<3, int>& p) {
			auto r = std::max<int>(0, std::min<int>(depth() - 1, p[2]));
			auto c = std::max<int>(0, std::min<int>(width() - 1, p[0]));

			return cells[r][c].elevation;
		})
	{
		std::vector<int8_t> v[3];

		{ // generate some entropy for use in the perlin noise functions
			std::default_random_engine generator;
			std::uniform_int_distribution<int> distribution(-127, 128);
			for (unsigned i = 2048; i--;)
			{
				v[0].push_back(distribution(generator));
				v[1].push_back(distribution(generator));
				v[2].push_back(distribution(generator));
			}
		}

		terrain = [v](const vec<3>& p) -> float
		{
			// TODO: make ground dip outside a set radius
			auto d = p[1]; // flat plane for now

			// d = p[0];

			d += g::gfx::noise::perlin(p, v[2]) * 0.5f;
			d += g::gfx::noise::perlin(p * 0.065, v[0]);
			d += std::min<float>(0, g::gfx::noise::perlin(p * 0.0234, v[1]) * 40);
			d += g::gfx::noise::perlin(p * 0.0123, v[2]) * 80;
			return d;
		};


		for (unsigned yi = 0; yi < d; yi++)
		{
			cells.push_back({});
			cells.back().resize(w);
		}

		unsigned block_side = sqrt(plant_block_size);

		auto blocks_r = ceil(depth() / block_side);
		auto blocks_c = ceil(width() / block_side);
		std::normal_distribution<float> norm(0.5f,0.2f);

		for (unsigned br = 0; br < blocks_r; br++)
		{
			for (unsigned bc = 0; bc < blocks_c; bc++)
			{
				auto r = br * block_side, c = bc * block_side;

				for (unsigned ri = r; ri < std::min<unsigned>(depth(), r + block_side); ri++)
				{
					for (unsigned ci = c; ci < std::min<unsigned>(width(), c + block_side); ci++)
					{
						cells[ri][ci].elevation += terrain(vec<3>{(float)ci, cells[ri][ci].elevation, (float)ri});
						cells[ri][ci].moisture(norm(rng));
						cells[ri][ci].plants(0);

						assert(isfinite<float>(cells[ri][ci].elevation));

						if (cells[ri][ci].is_active())
						{
							active_cells.push_back({ ri, ci });
						}
					}
				}
			}
		}

		std::uniform_int_distribution<int> oasis_start(0, active_cells.size() - 1);
		auto start = active_cells[oasis_start(rng)].cast<int>();

		cells[start[0]][start[1]].plants(0.5);
		cells[start[0]][start[1]].seed(0);

		{ // spawn around this cell
			std::uniform_int_distribution<int> plants(5, 15);
			std::uniform_int_distribution<int> spawn_range(-3, 3);
			auto max_dist = sqrt(2 * (3 * 3));

			for (auto i = plants(rng); i--;)
			{
				auto delta = vec<2, int>{spawn_range(rng), spawn_range(rng)};
				auto coord = start + delta;

				std::normal_distribution<float> density(1 - (delta.magnitude() / max_dist), 0.2);
				cells[coord[0]][coord[1]].plants(density(rng));
				cells[coord[0]][coord[1]].moisture(1);	
			}
		}

		camera.position = vec<3>{ (float)start[1], cells[start[0]][start[1]].elevation + 10, (float)start[0] };

		//for (unsigned r = 0; r < depth(); r++)
		//{
		//	for (unsigned c = 0; c < width(); c++)
		//	{
		//		cells[r][c].elevation = terrain(vec<3>{(float)c, 0, (float)r});

		//		if (cells[r][c].elevation >= 4)
		//		{
		//			active_cells.push_back({ r, c });
		//		}
		//	}
		//}

		std::cerr << "Cells that can host plants: " << active_cells.size() << std::endl;
	}

	inline size_t width() const { return cells[0].size(); }
	inline size_t depth() const { return cells.size(); }

	vec<2, int> idx2rc(int i) { 
		return { 
			static_cast<int>(i / width()), 
			static_cast<int>(i % width()) 
		}; 
	}
};

} // namespace game