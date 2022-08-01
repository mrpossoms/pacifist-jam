#ifdef GL_ES
precision mediump float;
#endif

in vec3 a_position;
in vec2 a_uv;
in vec3 a_normal;

uniform int u_block_width;
uniform int u_block_depth;
uniform vec2 u_start;
uniform vec3 u_positions[225];
// uniform float u_elevations[225];
uniform float u_densities[225];
uniform float u_fires[225];
uniform mat4 u_view;
uniform mat4 u_proj;
uniform float u_time;
uniform vec3 u_cam_pos;
uniform vec3 u_cam_vel;

out vec4 v_world_pos;
out vec4 v_screen_pos;
out vec3 v_normal;
out vec2 v_uv;
out float v_plant_density;
out float v_fire;

vec3 wave(vec3 p, float t)
{
   // float o = cos(p.y) + cos(p.z);
   vec3 w = vec3(
      cos(t + p.z - p.y),
      sin(t + p.x) * sin(t + p.z) * 0.1,
      sin(t + p.z + p.z)
   );

   return w;
}

vec3 wavy(vec3 p, float t)
{
   vec3 w0 = wave(p * 0.1, t + 0.33) * 0;
   vec3 w1 = wave(p * 0.33, t + 1.125) * 0.33;
   vec3 w2 = wave(p * 0.45, t + 0.125) * 0.033;   

   return (w0 + w1 + w2) * 0.25;
}

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

// vec3 pos_from_idx()
// {
// 	float r = gl_InstanceID / u_block_width;
// 	float c = gl_InstanceID % u_block_width;

// 	return vec3(u_start.x + r, u_elevations[gl_InstanceID], u_start.y + c);
// }

void main (void)
{


	vec3 pos = u_positions[gl_InstanceID]; // pos_from_idx();//
	vec3 vert_pos = ((a_position + vec3(0, 1, 0)) * vec3(1, 2, 1));

	vec3 wave_offset = wavy(pos, u_time) * vert_pos.y;
	vert_pos += wave_offset;

	v_world_pos = ((billboard(pos, u_view) * vec4(vert_pos, 0.0)) + vec4(pos, 1.0));
	vec4 view_pos = u_view * v_world_pos;

	float d = pow(distance(v_world_pos.xyz, u_cam_pos), 3);
	v_world_pos.xyz += u_cam_vel * (1.0 - vert_pos.y) * 10/d;

	v_screen_pos = u_proj * view_pos;
	gl_Position = v_screen_pos;

	v_plant_density = u_densities[gl_InstanceID];
	v_fire = u_fires[gl_InstanceID];
	v_uv = a_uv;

    // mat3 model_rot = mat3(normalize(u_model[0].xyz), normalize(u_model[1].xyz), normalize(u_model[2].xyz));
    v_normal = normalize(a_normal);
	// v_light_proj_pos = u_light_proj * u_light_view * v_world_pos;
	// v_light_proj_pos /= v_light_proj_pos.w;
	// v_light_proj_pos = (v_light_proj_pos + 1.0) / 2.0;
}
