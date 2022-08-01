#ifdef GL_ES
precision mediump float;
#endif

in vec2 v_uv;
in vec4 v_world_pos;
in float v_plant_density;
in float v_fire;

uniform sampler2D u_grass_tex;
uniform sampler2D u_bush_tex;
uniform sampler2D u_tree_tex;
uniform sampler2D u_fire_tex;
uniform float u_time;

out vec4 color;

vec4 flatten(vec4 background, vec4 foreground)
{
    return background * (1.0 - foreground.a) + foreground; 
}

mat3 transform(float s, vec2 t)
{
    return mat3(
        s, 0, 0,
        0, s, 0,
        t.x, t.y, 1
    );
}

vec2 growth(vec2 uv, float p)
{
    float w = 1 / p;
    return (transform(w, vec2((-w * 0.5) + (p * 0.5), -(w - 1))) * vec3(v_uv, 1.0)).xy;
}

void main (void)
{
    vec4 tree_color = texture(u_tree_tex, growth(v_uv, clamp((v_plant_density - 0.66) * 3, 0, 1)));
    vec4 bush_color = texture(u_bush_tex, growth(v_uv, clamp((v_plant_density * 2) - 0.5, 0, 1)));
    vec4 grass_color = texture(u_grass_tex, growth(v_uv, clamp(v_plant_density * 3, 0, 1)));

    color = flatten(tree_color, bush_color);
    color = flatten(color, grass_color);

    if (color.a <= 0.1) discard;

    float dim = 1.0 / (abs(v_world_pos.z) * 0.1);

    color.rgb *= clamp(dim, 0.5, 1.0);

    color += v_fire * texture(u_fire_tex, v_uv + vec2(0, u_time + v_world_pos.x + v_world_pos.z + sin(u_time + v_uv.x * 10) * 0.25)) * color.a;
}
