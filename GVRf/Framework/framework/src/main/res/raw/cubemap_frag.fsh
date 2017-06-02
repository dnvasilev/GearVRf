precision highp float;
uniform samplerCube u_texture;
in vec3 diffuse_coord;
out vec4 fragColor;

uniform vec3 u_color;
uniform float u_opacity;

void main()
{
  vec4 color = texture(u_texture, diffuse_coord);
  fragColor = vec4(color.r * u_color.r * u_opacity, color.g * u_color.g * u_opacity, color.b * u_color.b * u_opacity, color.a * u_opacity);
}
