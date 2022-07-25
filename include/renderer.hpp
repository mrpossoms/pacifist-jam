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
}

renderer(nj::state& s, const std::string& data_dir) : state(s), assets(data_dir)
{
	init_terrain();
}

void draw()
{
    terrain_mesh.using_shader(assets.shader("terrain.vs+terrain.fs"))
        .set_camera(state.camera)
        ["u_wall"].texture(assets.tex("cliff_wall_color.repeating.png", true))
        ["u_ground"].texture(assets.tex("earth_color.repeating.png", true))
        ["u_wall_normal"].texture(assets.tex("cliff_wall_normal.repeating.png", true))
        ["u_ground_normal"].texture(assets.tex("earth_normal.repeating.png", true))
        ["u_model"].mat4(mat<4, 4>::I())
        ["u_time"].flt(state.t)
        .draw<GL_TRIANGLES>();

    water_mesh.using_shader(assets.shader("water.vs+water.fs"))
        .set_camera(state.camera)
        ["u_wall"].texture(assets.tex("cliff_wall_color.repeating.png", true))
        ["u_ground"].texture(assets.tex("earth_color.repeating.png", true))
        ["u_wall_normal"].texture(assets.tex("cliff_wall_normal.repeating.png", true))
        ["u_ground_normal"].texture(assets.tex("earth_normal.repeating.png", true))
        ["u_model"].mat4(mat<4, 4>::I())
        ["u_time"].flt(state.t)
        .draw<GL_TRIANGLES>();

    g::gfx::debug::print(&state.camera).color({ 1, 0, 0, 1 }).ray(vec<3>{ 0, 0, 0 }, vec<3>{100, 0, 0});
    g::gfx::debug::print(&state.camera).color({ 0, 1, 0, 1 }).ray(vec<3>{ 0, 0, 0 }, vec<3>{0, 100, 0});
    g::gfx::debug::print(&state.camera).color({ 0, 0, 1, 1 }).ray(vec<3>{ 0, 0, 0 }, vec<3>{0, 0, 100});
}

};



} // namespace nj