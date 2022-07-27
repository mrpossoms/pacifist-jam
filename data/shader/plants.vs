#ifdef GL_ES
precision mediump float;
#endif

in vec3 a_position;
in vec2 a_uv;
in vec3 a_normal;

uniform vec3 u_positions[400];
uniform mat4 u_view;
uniform mat4 u_proj;

out vec4 v_screen_pos;
out vec3 v_normal;
out vec2 v_uv;

mat4 billboard(vec3 bb_pos, mat4 cam_view)
{
	vec3 u = vec3(0.0, 1.0, 0.0);
	vec3 f = normalize(vec3(cam_view[0].z, 0, cam_view[2].z));
	vec3 r = cross(f, u);

	return mat4(
		vec4(r, 0),
		vec4(u, 0),
		vec4(f, 0),
		vec4(0, 0, 0, 1)
	);
}

void main (void)
{
	vec3 pos = u_positions[gl_InstanceID];
	vec4 v_world_pos = vec4(pos, 1.0);
	v_screen_pos = u_proj * u_view * ((billboard(pos, u_view) * vec4(a_position, 0.0)) + v_world_pos);
	gl_Position = v_screen_pos;

	v_uv = a_uv;

    // mat3 model_rot = mat3(normalize(u_model[0].xyz), normalize(u_model[1].xyz), normalize(u_model[2].xyz));
    v_normal = normalize(a_normal);
	// v_light_proj_pos = u_light_proj * u_light_view * v_world_pos;
	// v_light_proj_pos /= v_light_proj_pos.w;
	// v_light_proj_pos = (v_light_proj_pos + 1.0) / 2.0;
}
