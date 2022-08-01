#include "g.h"
#include "state.hpp"
#include "renderer.hpp"
#include "gameplay.hpp"

struct njord : public g::core
{

	g::asset::store assets;
	nj::state state;
    std::unique_ptr<nj::renderer> renderer;

    std::string data_dir = "data";

	njord(int argc, const char* argv[]) : state(300, 300)
	{
        if (argc > 1)
        {
            data_dir = std::string(argv[1]);
        }
	}

	~njord() = default;

	virtual bool initialize()
	{
        renderer = std::make_unique<nj::renderer>(state, data_dir);

        // Setup camera input
        auto& cam = state.camera;
        cam.drag = 1;
        cam.speed = 20;
        cam.on_input = [](nj::flying_cam& cam, float dt) {
            static double xlast, ylast;
            float sensitivity = 0.5f;
            double xpos = 0, ypos = 0;
            auto mode = glfwGetInputMode(g::gfx::GLFW_WIN, GLFW_CURSOR);

            if (GLFW_CURSOR_DISABLED == mode)
            {
                glfwGetCursorPos(g::gfx::GLFW_WIN, &xpos, &ypos);
            }

            if (glfwGetInputMode(g::gfx::GLFW_WIN, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
                if (xlast != 0 || ylast != 0)
                {
                    auto dx = xpos - xlast;
                    auto dy = ypos - ylast;
                    cam.d_pitch(dy * dt * sensitivity);
                    cam.d_roll(dx * dt * sensitivity);
                   
                }

            xlast = xpos; ylast = ypos;

            auto speed = cam.speed * dt;
            cam.velocity += cam.forward() * speed;
            if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) speed *= (cam.touching_surface ? 5 : 1);
            if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_Q) == GLFW_PRESS) cam.d_yaw(-speed * dt);
            if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_E) == GLFW_PRESS) cam.d_yaw(speed * dt);
            if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_W) == GLFW_PRESS) cam.velocity += cam.forward() * speed;
            if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_S) == GLFW_PRESS) cam.velocity += cam.forward() * -speed * 0.5;
            if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_A) == GLFW_PRESS) cam.velocity += cam.left() * -speed;
            if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_D) == GLFW_PRESS) cam.velocity += cam.left() * speed;
            if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_SPACE) == GLFW_PRESS) cam.velocity += cam.up() * 5 * cam.touching_surface;
            if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetInputMode(g::gfx::GLFW_WIN, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            if (glfwGetMouseButton(g::gfx::GLFW_WIN, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            {
                glfwSetInputMode(g::gfx::GLFW_WIN, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        };

        glfwSetInputMode(g::gfx::GLFW_WIN, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		return true;
	}

	virtual void update(float dt)
	{
		glClearColor(0.5, 0.5, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // process input and update the velocities.
        state.camera.pre_update(dt, 0);

        g::game::heightmap_collider terrain_collider(state.terrain_hm);
        auto intersections = state.camera.intersections(terrain_collider, 1);
        state.camera.touching_surface = intersections.size() > 0;

        // resolve collisions. This modifies velocities so that the camera will not penetrate
        // the surface we are colliding with (ground).
        if (state.camera.touching_surface)
        {
            std::cerr << "touching" << std::endl;
            g::dyn::cr::resolve_linear<nj::flying_cam>(state.camera, intersections);
        }

        step_world(state, 1);

        // after velocities have been corrected, update the camera's position
        state.camera.update(dt, 0);

        renderer->draw();
        auto twoD = state.camera.position + vec<3>{0, 0, 2};
        twoD[1] = state.terrain_hm.elevation_at(twoD);
        auto n = (g::game::normal_from(state.terrain_hm, twoD) + vec<3>{1, 1, 1}) * 0.5f;
        // std::cerr<< n.to_string() << std::endl;
        g::gfx::debug::print(&state.camera).color({ n[0], n[1], n[2], 1 }).ray(twoD, n);

        state.t += dt;
        state.frame += 1;
	}

};

#ifdef __EMSCRIPTEN__
EM_JS(int, canvas_get_width, (), {
  return document.getElementById('canvas').width;
});

EM_JS(int, canvas_get_height, (), {
  return document.getElementById('canvas').height;
});
#endif

int main (int argc, const char* argv[])
{
	njord game(argc, argv);

	g::core::opts opts;

	opts.name = "Njord";
	opts.gfx.fullscreen = false;

#ifdef __EMSCRIPTEN__
	auto monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	opts.gfx.width = canvas_get_width();
	opts.gfx.height = canvas_get_height();
#else
	opts.gfx.fullscreen = false;
#endif

	game.start(opts);

	return 0;
}
