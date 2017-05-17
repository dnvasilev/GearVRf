
precision highp float;
in vec3 a_position;
in vec3 a_normal;

$TRANSFORM_UBO

out vec3 v_viewspace_position;
out vec3 v_viewspace_normal;
void main()
{
  vec4 v_viewspace_position_vec4 = u_mv * vec4(a_position,1.0);
  v_viewspace_position = v_viewspace_position_vec4.xyz / v_viewspace_position_vec4.w;
  v_viewspace_normal = (u_mv_it * vec4(a_normal, 1.0)).xyz;
  gl_Position = u_mvp * vec4(a_position, 1.0);
 }