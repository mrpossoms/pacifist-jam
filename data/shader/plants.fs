#ifdef GL_ES
precision mediump float;
#endif

in vec2 v_uv;

uniform sampler2D u_plants;

out vec4 color;

void main (void)
{
    color = texture(u_plants, v_uv);

    if (color.a <= 0.1) discard;
}
