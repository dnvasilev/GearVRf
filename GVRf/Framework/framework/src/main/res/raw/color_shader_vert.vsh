precision mediump float;
in vec3 a_position;
@MATRIX_UNIFORMS
void main()
{
   gl_Position = u_mvp * vec4(a_position, 1);
 }