#include "g.h"
#include "state.hpp"
#include "renderer.hpp"

struct njord : public g::core
{

	g::asset::store assets;
	nj::state state;
    std::unique_ptr<nj::renderer> renderer;

    std::string data_dir = "data";

	njord(int argc, const char* argv[]) : state(400, 400)
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
        cam.position = { state.width() / 2.f, 10, state.depth() / 2.f };
        cam.gravity = {0, 0, 0};
        cam.on_input = [](fps_camera& cam, float dt) {
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
                    cam.pitch += (dy * dt * sensitivity);
                    cam.yaw += (-dx * dt * sensitivity);
                }

            xlast = xpos; ylast = ypos;

            auto speed = cam.speed;
            speed *= cam.touching_surface ? 1 : 0.1;
            if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) speed *= (cam.touching_surface ? 5 : 1);
            if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_W) == GLFW_PRESS) cam.velocity += cam.forward() * speed;
            if (glfwGetKey(g::gfx::GLFW_WIN, GLFW_KEY_S) == GLFW_PRESS) cam.velocity += cam.forward() * -speed;
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

        // todo: collision res

        // after velocities have been corrected, update the camera's position
        state.camera.update(dt, 0);

        renderer->draw();

        state.t += dt;
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
