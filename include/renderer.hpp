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
std::vector<float> plant_elevations;
std::vector<float> plant_densities;
std::vector<float> plant_fires;

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
        plant_densities.resize(state.active_cells.size());
        plant_elevations.resize(state.active_cells.size());
        plant_fires.resize(state.active_cells.size());
        for (unsigned i = 0; i < state.active_cells.size(); i++)
        {
            auto r = state.active_cells[i][0];
            auto c = state.active_cells[i][1];
            auto& cell = state.cells[r][c];
            plant_elevations[i] = cell.elevation;
            plant_positions[i] = vec<3>{ (float)r, cell.elevation, (float)c } + nj::random_norm_vec<3>(state.rng) * vec<3>{1, 1, 1} + vec<3>{0, 0, 0};
            plant_densities[i] = cell.plants();
            plant_fires[i] = cell.fire();
        }
    }

    // I hate this. Should reorganize state in a DOD fashion so that density states
    // can be sent along to the gpu directly
    for (unsigned i = 0; i < state.active_cells.size(); i++)
    {
        auto r = state.active_cells[i][0];
        auto c = state.active_cells[i][1];
        auto& cell = state.cells[r][c];
        plant_densities[i] = cell.plants();
        plant_fires[i] = cell.fire();
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

    constexpr auto plant_batch = 225;
    auto& tree_tex = assets.tex("tree.clamped.png", true);
    auto& bush_tex = assets.tex("bush.clamped.png", true);
    auto& grass_tex = assets.tex("grass.clamped.png", true);
    auto& fire_tex = assets.tex("fire_color.repeating.png", true);
    auto& plant_shaders = assets.shader("plants.vs+plants.fs");
    auto& debug_shaders = assets.shader("plants.vs+uvs.fs");
    for (int i = 0; i < (int)plant_positions.size();)
    {
        auto di = std::min<unsigned>(plant_batch, plant_positions.size() - i);
        auto& middle = plant_positions[i + (di >> 1)];
        // TODO: organize the positions in blocks so that they can be culled more easily
        //if ((middle - state.camera.position).dot(state.camera.forward()) > 0)
        {
            billboard_mesh.using_shader(plant_shaders)
                .set_camera(state.camera)
                //["u_elevations"].fltn(plant_elevations.data() + i, di)
                ["u_positions"].vec3n(plant_positions.data() + i, di)
                ["u_densities"].fltn(plant_densities.data() + i, di)
                ["u_fires"].fltn(plant_fires.data() + i, di)
                ["u_block_width"].int1(sqrt(state.plant_block_size))
                ["u_block_depth"].int1(sqrt(state.plant_block_size))
                ["u_grass_tex"].texture(grass_tex)
                ["u_bush_tex"].texture(bush_tex)
                ["u_tree_tex"].texture(tree_tex)
                ["u_fire_tex"].texture(fire_tex)
                ["u_cam_pos"].vec3(state.camera.position)
                ["u_cam_vel"].vec3(state.camera.velocity)
                ["u_time"].flt(state.t)
                .draw<GL_TRIANGLE_FAN>(di);

            // billboard_mesh.using_shader(debug_shaders)
            //     .set_camera(state.camera)
            //     ["u_positions"].vec3n(plant_positions.data() + i, batch)
            //     ["u_densities"].fltn(plant_densities.data() + i, batch)
            //     ["u_grass_tex"].texture(grass_tex)
            //     ["u_bush_tex"].texture(bush_tex)
            //     ["u_tree_tex"].texture(tree_tex)
            //     ["u_time"].flt(state.t)
            //     .draw<GL_LINES>(batch);

        }

        
        i += di;
    }


    auto seed_batch = 512;
    auto& seed_tex = assets.tex("seed_color.png", true);
    auto& seed_shaders = assets.shader("seeds.vs+textured.fs");
    for (int i = 0; i < state.seed_positions.size();)
    {
        auto di = std::min<unsigned>(seed_batch, state.seed_positions.size() - i);

        // TODO: organize the positions in blocks so that they can be culled more easily
        //if ((middle - state.camera.position).dot(state.camera.forward()) > 0)
        {
            billboard_mesh.using_shader(seed_shaders)
                .set_camera(state.camera)
                ["u_positions"].vec3n(state.seed_positions.list() + i, di)
                ["u_tex"].texture(seed_tex)
                ["u_time"].flt(state.t)
                .draw<GL_TRIANGLE_FAN>(di);

            // billboard_mesh.using_shader(debug_shaders)
            //     .set_camera(state.camera)
            //     ["u_positions"].vec3n(plant_positions.data() + i, batch)
            //     ["u_densities"].fltn(plant_densities.data() + i, batch)
            //     ["u_grass_tex"].texture(grass_tex)
            //     ["u_bush_tex"].texture(bush_tex)
            //     ["u_tree_tex"].texture(tree_tex)
            //     ["u_time"].flt(state.t)
            //     .draw<GL_LINES>(batch);

        }


        i += di;
    }

    water_mesh.using_shader(assets.shader("water.vs+water.fs"))
        .set_camera(state.camera)
        ["u_wall"].texture(assets.tex("water_color.repeating.png", true))
        ["u_ground"].texture(assets.tex("white_cap_color.repeating.png", true))
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