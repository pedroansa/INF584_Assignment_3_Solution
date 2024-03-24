#version 410 core // Minimal GL version support expected from the GPU

const float PI = 3.1415926535897932384626433832795;
const int MAX_NUM_OF_LIGHT_SOURCES = 8;

struct LightSource {
	vec3 position;
	vec3 color;
	float intensity;
};

struct Material {
	vec3 albedo;
	float roughness;
	float metallicness;
};

//ShadowMAP PART
uniform sampler2D shadowMap[3];

uniform mat4 viewMat;
uniform int numOfLightSources;
uniform LightSource lightSourceSet[MAX_NUM_OF_LIGHT_SOURCES];
uniform Material material;

in vec3 fNormal; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
in vec3 fPosition; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
out vec4 colorResponse; // Shader output: the color response attached to this fragment

//Shadow
in vec4 lightsPos[3];

bool isShadowed(vec4 fragPosLightSpace, sampler2D shadowmap) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowmap, projCoords.xy).r;
    float currentDepth = projCoords.z - 0.05;
    return currentDepth > closestDepth;
}


float shadowPCF(sampler2D shadowMap, vec4 fragPosLightSpace, float shadowBias, vec2 texelSize, int filterSize) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // Transform to [0,1] space
    float currentDepth = projCoords.z - shadowBias;

    float shadow = 0.0;
    vec2 texelStep = texelSize / textureSize(shadowMap, 0); // Get the size of one texel

    // Loop over the kernel
    for (int x = -filterSize; x <= filterSize; ++x) {
        for (int y = -filterSize; y <= filterSize; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelStep).r;
            shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
        }
    }

    // Average the shadows
    int samples = (filterSize * 2 + 1) * (filterSize * 2 + 1);
    shadow /= float(samples);

    return shadow;
}

float VSMShadowCalculation(sampler2D shadowMap, vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    vec2 moments = texture(shadowMap, projCoords.xy).rg;
    float depth = projCoords.z;
    float bias = 0.9; // Consider adjusting this based on your scene.
    float variance = moments.y - (moments.x * moments.x);
    variance = max(variance, 0.1); // Ensures a minimum variance to avoid division by zero.

    float d = depth + bias - moments.x;
    float pMax = variance / (variance + d * d * d);

    float lightBleedReduction = 0.2; 
    pMax = mix(pMax, 1.0, lightBleedReduction);

    return pMax;
}


vec3 toneMap (vec3 radiance, float exposure, float gamma) {
	vec3 rgb = exposure * radiance;
	return vec3 (pow (rgb.r, gamma), 
				 pow (rgb.g, gamma),
				 pow (rgb.b, gamma));
}

vec3 attenuation (LightSource l, vec3 lightPosition, vec3 p) {
	float d = distance (lightPosition, p);
	return l.intensity*l.color/(d*d);
}

vec3 diffuseBRDF (Material m) {
	return (1.0 - m.metallicness) * m.albedo/PI;
}

float D_GGX(float NoH, float a) {
    float a2 = a * a;
    float f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2 / (PI * f * f);
}

vec3 F_Schlick(float u, vec3 f0) {
	return f0 + (vec3(1.0) - f0) * pow(1.0 - u, 5.0);
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a) {
    float a2 = a * a;
    float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / (GGXV + GGXL);
}

vec3 microfacetBRDF(Material m, vec3 n, vec3 v, vec3 l)
{
	vec3 h = normalize(v + l);
    float NoV = abs(dot(n, v)) + 1e-5;
    float NoL = clamp(dot(n, l), 0.0, 1.0);
    float NoH = clamp(dot(n, h), 0.0, 1.0);
    float LoH = clamp(dot(l, h), 0.0, 1.0);

    // perceptually linear roughness to roughness (see parameterization)
    float roughness = m.roughness * m.roughness;

    float D = D_GGX(NoH, roughness);
    float reflectance  = 1.0;
    vec3 f0 = 0.16 * reflectance * reflectance * (1.0 - m.metallicness) + m.albedo * m.metallicness;
    vec3  F = F_Schlick(LoH, f0);
    float V = V_SmithGGXCorrelated(NoV, NoL, roughness);
    return D*V*F;
}

void main () {
	vec3 radiance = vec3 (0.0);
	vec3 n = normalize (fNormal);
	vec3 wo = normalize (-fPosition);
	float c[3];

    float shadowBias = 0.5;
    vec2 texelSize = vec2(1); // Adjust based on your shadow map resolution
    int filterSize = 3; // Increase for softer shadows

    // Replace the shadow calculation with:
    float shadow = 0.0;

	for (int i = 0; i < min (1, numOfLightSources); ++i) {
		c[i] = 0.f;// texture(shadowMap[i], 0.5, 0.).r;
			
		LightSource l = lightSourceSet[i];
		vec3 lightPosition = vec3 (viewMat * vec4 (l.position, 1.0));
		vec3 wi = normalize (lightPosition - fPosition);
		vec3 li = attenuation (l, lightPosition, fPosition);
		vec3 fd = diffuseBRDF (material);
		vec3 fs = microfacetBRDF (material, n, wo, wi);
        float shadowFactor = VSMShadowCalculation(shadowMap[i], lightsPos[i]);
    
		vec3 fr = fd+fs;
        if(shadowFactor < 0.2){
            fr = fd;
        }
		float nDotL = max (0.0, dot( n, wi));
        
        radiance += (1-shadowFactor) * li * fr * nDotL;
	}
	//radiance = toneMap (radiance, 1.0, 1.0);
	//colorResponse = vec4 (c[0], c[1], c[2], 1.0);
	colorResponse = vec4(radiance, 1.0);
}