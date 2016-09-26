#ifdef HAS_diffuseTexture
uniform sampler2D diffuseTexture;
#else
uniform sampler2D u_texture;
#endif

uniform vec3 u_color;
uniform float u_opacity;
uniform vec4 ambient_color;
uniform vec4 diffuse_color;
uniform vec4 specular_color;
uniform vec4 emissive_color;
uniform float specular_exponent;

struct Surface
{
   vec3 viewspaceNormal;
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
   vec4 emission;
};

Surface @ShaderName()
{
	vec4 diffuse = u_color;
#ifdef HAS_LIGHTSOURCES
    diffuse *= diffuse_color;
#endif
#ifdef HAS_diffuseTexture
	diffuse *= texture(diffuseTexture, diffuse_coord.xy);
#else
    diffuse *= texture(u_texture, diffuse_coord.xy);
#endif
    diffuse = vec4(diffuse.r * u_opacity, diffuse.g * u_opacity, diffuse.b * u_opacity, diffuse.a * u_opacity);
	return Surface(viewspace_normal, ambient_color, diffuse, specular_color, emissive_color);
}
