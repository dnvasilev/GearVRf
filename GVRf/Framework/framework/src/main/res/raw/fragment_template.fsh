precision highp float;

@MATRIX_UNIFORMS

in vec3 viewspace_position;
in vec3 viewspace_normal;
in vec4 local_position;
in vec4 proj_position;
in vec3 view_direction;
in vec2 diffuse_coord;
out vec4 fragColor;

#ifdef HAS_ambientTexture
out vec2 ambient_coord;
#endif

#ifdef HAS_specularTexture
out vec2 specular_coord;
#endif

#ifdef HAS_emissiveTexture
out vec2 emissive_coord;
#endif

#ifdef HAS_normalTexture
out vec2 normal_coord;
#endif

#ifdef HAS_SHADOWS
uniform lowp sampler2DArray u_shadow_maps;

float unpackFloatFromVec4i(const vec4 value)
{
    const vec4 bitSh = vec4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
    const vec4 unpackFactors = vec4( 1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0 );
    return dot(value,unpackFactors);
}

#endif

struct Radiance
{
   vec3 ambient_intensity;
   vec3 diffuse_intensity;
   vec3 specular_intensity;
   vec3 direction; // view space direction from light to surface
   float attenuation;
};

@FragmentSurface

@FragmentAddLight

@LIGHTSOURCES

void main()
{
	Surface s = @ShaderName();
#if defined(HAS_LIGHTSOURCES)
    vec4 color = LightPixel(s);
	color = clamp(color, vec4(0), vec4(1));
	fragColor = color;
#else
	fragColor = s.diffuse;
	//fragColor = vec4(1,0,0,1);
#endif
}
