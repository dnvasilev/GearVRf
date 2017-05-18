
precision highp float;
uniform samplerCube u_texture;

layout (std140) uniform Material_ubo
{
    vec3 u_color;
    float u_opacity;
};

$TRANSFORM_UBO

in vec3 v_viewspace_position;
in vec3 v_viewspace_normal;
out vec4 outColor;

void main()
{
  vec3 v_reflected_position = reflect(v_viewspace_position, normalize(v_viewspace_normal));
  vec3 v_tex_coord = (u_view_i * vec4(v_reflected_position, 1.0)).xyz;
  v_tex_coord.z = -v_tex_coord.z;
  vec4 color = texture(u_texture, v_tex_coord.xyz);
  outColor = vec4(color.r * u_color.r * u_opacity, color.g * u_color.g * u_opacity, color.b * u_color.b * u_opacity, color.a * u_opacity);
}
