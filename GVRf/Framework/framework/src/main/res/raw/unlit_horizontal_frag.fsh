precision highp float;
uniform sampler2D u_texture;

uniform vec3 u_color;
uniform float u_opacity;
uniform mat4 u_mvp;
uniform vec4 u_right;

in vec2 diffuse_coord;
out vec4 outColor;

void main()
{
    vec2 tex_coord = vec2(0.5 * (diffuse_coord.x + float(u_right.x)), diffuse_coord.y);
    vec4 color = texture(u_texture, tex_coord);
    outColor = vec4(color.r * u_color.r * u_opacity, color.g * u_color.g * u_opacity, color.b * u_color.b * u_opacity, color.a * u_opacity);
}
