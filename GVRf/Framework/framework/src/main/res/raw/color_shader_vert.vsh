precision mediump float;
in vec3 a_position;
$TRANSFORM_UBO
void main()
{
   gl_Position = u_mvp * vec4(a_position, 1);
 }