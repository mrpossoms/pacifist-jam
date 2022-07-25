#ifdef GL_ES
precision mediump float;
#endif

in vec3 a_position;
in vec3 a_normal;
in vec3 a_tangent;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform float u_time;

out vec3 v_normal;
out vec3 v_tangent;
out vec3 v_up;
out vec4 v_world_pos;
out mat3 v_basis;

vec3 wave(vec3 p, float t)
{
   // float o = cos(p.y) + cos(p.z);
   vec3 w = vec3(
      cos(t + p.z - p.y) * 0.25,
      sin(t + p.x) * sin(t + p.z),
      sin(t + p.z + p.z) * 0.25
   );

   return w;
}

vec3 wavy(vec3 p, float t)
{
   vec3 w0 = wave(p * 0.1, t + 0.33) * 3.33;
   vec3 w1 = wave(p * 0.33, t + 1.125) * 1.1;
   vec3 w2 = wave(p * 0.45, t + 0.125) * 1;   
   vec3 w3 = wave(p * 0.66, t + 3) * 0.66;
   vec3 w4 = wave(p * 1.66, t + 1) * 0.222;

   return p + (w0 + w1 + w2 + w3 + w4) * 0.5;
}

void main (void)
{
   vec3 position = wavy(a_position, u_time);

   v_world_pos = u_model * vec4(position, 1.0);
   gl_Position = u_proj * u_view * v_world_pos;

    mat3 model_rot = mat3(normalize(u_model[0].xyz), normalize(u_model[1].xyz), normalize(u_model[2].xyz));
    v_normal = normalize(model_rot * a_normal);
    v_tangent = normalize(model_rot * a_tangent);
    v_up = normalize(position);

    v_basis = mat3(v_tangent, v_normal, cross(v_tangent, v_normal));
}
