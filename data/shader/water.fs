#ifdef GL_ES
precision mediump float;
#endif

in vec4 v_world_pos;
in vec3 v_normal;
in vec3 v_tangent;
in vec3 v_up;
in mat3 v_basis;

uniform sampler2D u_wall;
uniform sampler2D u_wall_normal;
uniform sampler2D u_ground;
uniform sampler2D u_ground_normal;
uniform float u_time;

out vec4 color;

void main (void)
{
    vec2 uv = v_world_pos.xz * 0.25;
    float w = pow(clamp(v_world_pos.y, 0, 1), 32.0);

    vec4 wall_color = texture(u_wall, uv);
    vec4 ground_color = texture(u_ground, uv);

    // color = vec4(1.0);
    color = mix(wall_color, ground_color, w);

    //color.rgb *= (dot(v_basis * textel_norm, light_dir) + 1) * 0.5;
    // color.rgb += vec3(0.5);
    // color.rgb *= vec3(0.25, 0.25, 1.0);
    color.a = 0.75;
    // color.rgb *= dot(v_normal, light_dir);
    // color.rgb = (textel_norm + 1.0) * 0.5;
}
