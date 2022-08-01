#ifdef GL_ES
precision mediump float;
#endif

in vec3 a_position;
in vec2 a_uv;
in vec3 a_normal;

uniform vec3 u_positions[512];
uniform mat4 u_view;
uniform mat4 u_proj;
uniform float u_time;

out vec4 v_world_pos;
out vec4 v_screen_pos;
out vec3 v_normal;
out vec2 v_uv;

void main (void)
{
	vec3 pos = u_positions[gl_InstanceID];

	v_world_pos = (u_view * vec4(pos, 1.0)) + vec4(a_position * 0.2, 0.0);
	v_screen_pos = u_proj * v_world_pos;
	gl_Position = v_screen_pos;

	v_uv = a_uv;

    // mat3 model_rot = mat3(normalize(u_model[0].xyz), normalize(u_model[1].xyz), normalize(u_model[2].xyz));
    v_normal = normalize(a_normal);
	// v_light_proj_pos = u_light_proj * u_light_view * v_world_pos;
	// v_light_proj_pos /= v_light_proj_pos.w;
	// v_light_proj_pos = (v_light_proj_pos + 1.0) / 2.0;
}
