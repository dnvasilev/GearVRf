
in vec3 a_position;

$TRANSFORM_UBO

void main()
{
  vec4 pos = vec4(a_position, 1.0);
  gl_Position = u_mvp * pos;
}