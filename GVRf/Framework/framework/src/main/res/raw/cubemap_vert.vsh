
in vec3 a_position;
out vec3 diffuse_coord;

@MATRIX_UNIFORMS

void main()
{
  vec4 pos = vec4(a_position, 1.0);
  diffuse_coord = normalize((u_model * pos).xyz);
  diffuse_coord.z = -diffuse_coord.z;
  gl_Position = u_mvp * pos;
}
