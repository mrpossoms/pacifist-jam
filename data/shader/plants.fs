#ifdef GL_ES
precision mediump float;
#endif

in vec2 v_uv;
in vec4 v_world_pos;

uniform sampler2D u_grass_tex;
uniform sampler2D u_bush_tex;
uniform sampler2D u_tree_tex;
uniform float u_time;

out vec4 color;

vec4 flatten(vec4 background, vec4 foreground)
{
    return background * (1.0 - foreground.a) + foreground; 
}

void main (void)
{
    vec2 uv = v_uv * ((sin(u_time) + 1.0));

    vec4 tree_color = texture(u_tree_tex, uv);
    vec4 bush_color = texture(u_bush_tex, uv);
    vec4 grass_color = texture(u_grass_tex, uv);

    color = flatten(tree_color, bush_color);
    color = flatten(color, grass_color);

    if (color.a <= 0.1) discard;

    float dim = 1.0 / (abs(v_world_pos.z) * 0.1);

    color.rgb *= clamp(dim, 0.5, 1.0);
}
