#pragma once
#include <g.h>

namespace nj
{
	
struct flying_cam : public camera_perspective, updateable, ray_collider
{
    vec<3> velocity = {};       
    float drag = 0.3;

    bool touching_surface = false;

    std::function<void(flying_cam& cam, float dt)> on_input;

    float speed = 1;

    std::vector<ray>& rays() override
    {
        ray_list.clear();
        ray_list.push_back({ position + forward() * 2, dir });

        return ray_list;
    }

    void pre_update(float dt, float time) override
    {
        if (on_input) on_input(*this, dt);

        velocity += -velocity * drag * dt;
        dir = velocity * dt;

    }

    void update(float dt, float t) override
    {
        //velocity += gravity * dt;

        position += velocity * dt;
    }
private:
    vec<3> dir; //< used for ray generation in collision detection
};


}