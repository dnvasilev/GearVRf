in vec3 a_position;
in vec2 a_texcoord;
out vec2 diffuse_coord;

@MATRIX_UNIFORMS

void main()
{
  vec4 pos = vec4(a_position, 1.0);
  diffuse_coord = vec2(a_texcoord.x, 1.0 - a_texcoord.y);
  gl_Position = u_mvp * pos;
}