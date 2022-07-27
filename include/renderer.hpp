#pragma once

namespace nj
{

namespace vertex
{

} // namespace nj::vertex

struct renderer
{

g::asset::store assets;
nj::state& state;
g::gfx::mesh<g::gfx::vertex::pos_norm_tan> terrain_mesh, water_mesh;
g::gfx::mesh<g::gfx::vertex::pos_uv_norm> billboard_mesh;
std::vector<vec<3>> plant_positions;

void init_terrain()
{
    //auto terrain_generator = [](const g::game::sdf& sdf, const vec<3>& pos) -> g::gfx::vertex::pos_norm_tan
    //{
    //    g::gfx::vertex::pos_norm_tan v;

    //    const float s = 1;
    //    v.normal = normal_from_sdf(sdf, pos, s);
    //    v.position = pos;

    //    if (fabs(v.normal.dot({0, 1, 0})) > 0.999f)
    //    {       
    //        v.tangent = vec<3>::cross(v.normal, {1, 0, 0});
    //    }
    //    else
    //    {
    //        v.tangent = vec<3>::cross(v.normal, {0, 1, 0});   
    //    }
    //
    //    return v;
    //};

    //vec<3> corners[] = {
    //    {0, -1, 0},
    //    {(float)state.width(), 10, (float)state.depth()},
    //};

    //terrain_mesh = g::gfx::mesh_factory::from_sdf< g::gfx::vertex::pos_norm_tan>(state.terrain, terrain_generator, corners, 7);
    terrain_mesh = g::gfx::mesh_factory::from_heightmap<g::gfx::vertex::pos_norm_tan>(
        [&](int x, int y) -> g::gfx::vertex::pos_norm_tan {
            const auto& cell = state.cells[x][y];

            g::gfx::vertex::pos_norm_tan vert_out;
            vert_out.position = vec<3>{ (float)x, cell.elevation, (float)y };
            //vert_out.normal = 

            return vert_out;
        },
        state.width(),
        state.depth()
    );

    water_mesh = g::gfx::mesh_factory::from_heightmap<g::gfx::vertex::pos_norm_tan>(
        [&](int x, int y) -> g::gfx::vertex::pos_norm_tan {
            const auto& cell = state.cells[x][y];

            g::gfx::vertex::pos_norm_tan vert_out;
            vert_out.position = vec<3>{ (float)x, 0, (float)y };
            //vert_out.normal = 

            return vert_out;
        },
        state.width(),
            state.depth()
            );

    billboard_mesh = g::gfx::mesh_factory::plane();
}

renderer(nj::state& s, const std::string& data_dir) : state(s), assets(data_dir)
{
	init_terrain();
}

void draw()
{
    if (plant_positions.size() != state.active_cells.size())
    {
        plant_positions.resize(state.active_cells.size());
        for (unsigned i = 0; i < state.active_cells.size(); i++)
        {
            auto r = state.active_cells[i][0];
            auto c = state.active_cells[i][1];
            auto& cell = state.cells[r][c];
            plant_positions[i] = vec<3>{ (float)r, cell.elevation, (float)c } + nj::random_norm_vec<3>(state.rng);
        }
    }

    terrain_mesh.using_shader(assets.shader("terrain.vs+terrain.fs"))
        .set_camera(state.camera)
        ["u_wall"].texture(assets.tex("earth_color.repeating.png", true))
        ["u_wall_normal"].texture(assets.tex("earth_normal.repeating.png", true))
        ["u_sand"].texture(assets.tex("sand.repeating.png", true))
        ["u_sand_normal"].texture(assets.tex("sand_normal.repeating.png", true))
        ["u_ground"].texture(assets.tex("earth_color.repeating.png", true))
        ["u_ground_normal"].texture(assets.tex("earth_normal.repeating.png", true))
        ["u_model"].mat4(mat<4, 4>::I())
        ["u_time"].flt(state.t * 0)
        .draw<GL_TRIANGLES>();

    glDisable(GL_CULL_FACE);

    constexpr auto batch = 400;
    for (unsigned i = 0; i < plant_positions.size();)
    {
        auto& middle = plant_positions[i + (batch >> 1)];
        // TODO: organize the positions in blocks so that they can be culled more easily
        //if ((middle - state.camera.position).dot(state.camera.forward()) > 0)
        {
            g::gfx::debug::print(&state.camera).color({ 1, 1, 1, 1 }).ray(middle, vec<3>{0, 10, 0});
            billboard_mesh.using_shader(assets.shader("plants.vs+plants.fs"))
                .set_camera(state.camera)
                ["u_positions"].vec3n(plant_positions.data() + i, batch)
                ["u_plants"].texture(assets.tex("shitty_grass_color.png", true))
                .draw<GL_TRIANGLE_FAN>(batch);

        }

        
        i += std::min<unsigned>(batch, plant_positions.size() - i);
    }


    water_mesh.using_shader(assets.shader("water.vs+water.fs"))
        .set_camera(state.camera)
        ["u_wall"].texture(assets.tex("cliff_wall_color.repeating.png", true))
        ["u_ground"].texture(assets.tex("earth_color.repeating.png", true))
        ["u_wall_normal"].texture(assets.tex("cliff_wall_normal.repeating.png", true))
        ["u_ground_normal"].texture(assets.tex("earth_normal.repeating.png", true))
        ["u_model"].mat4(mat<4, 4>::I())
        ["u_time"].flt(state.t)
        .draw<GL_TRIANGLES>();
    glEnable(GL_CULL_FACE);

    g::gfx::debug::print(&state.camera).color({ 1, 0, 0, 1 }).ray(vec<3>{ 0, 0, 0 }, vec<3>{100, 0, 0});
    g::gfx::debug::print(&state.camera).color({ 0, 1, 0, 1 }).ray(vec<3>{ 0, 0, 0 }, vec<3>{0, 100, 0});
    g::gfx::debug::print(&state.camera).color({ 0, 0, 1, 1 }).ray(vec<3>{ 0, 0, 0 }, vec<3>{0, 0, 100});
}

};



} // namespace nj