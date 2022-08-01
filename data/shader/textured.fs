#ifdef GL_ES
precision mediump float;
#endif

in vec2 v_uv;

uniform sampler2D u_tex;
out vec4 color;

void main (void)
{
    color = texture(u_tex, v_uv);

    if (color.a <= 0.1) discard;
}
