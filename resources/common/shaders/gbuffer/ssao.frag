#version 330

// Input: UV coordinates
in INTERFACE {
	vec2 uv;
} In ; ///< vec2 uv;

layout(binding = 0) uniform sampler2D depthTexture; ///< Depth texture.
layout(binding = 1) uniform sampler2D normalTexture; ///< Normal texture.
layout(binding = 2) uniform sampler2D noiseTexture; ///< 5x5 3-components noise texture with float precision.

uniform mat4 projectionMatrix; ///< The camera projection parameters.
uniform vec3 samples[24]; ///< Unique sample directions on a sphere.

#define RADIUS 0.5 ///< The sampling radius.

layout(location = 0) out float fragColor; ///< SSAO.

/** Linearize a NDC space to view space using the camera projection parameters. 
\param depth the non-linear depth
\return the linear depth
*/
float linearizeDepth(float depth){
	float depth2 = 2.0*depth-1.0; // Move from [0,1] to [-1,1].
	float viewDepth = - projectionMatrix[3][2] / (depth2 + projectionMatrix[2][2] );
	return viewDepth;
}

/** Estimate the view space position of a given fragment.
\param uv The texture coordinates of the fragment
\return the view space position
*/
vec3 positionFromUV(vec2 uv){
	// Linearize depth -> in view space.
	float depth = texture(depthTexture, uv).r;
	float viewDepth = linearizeDepth(depth);
	// Compute the x and y components in view space.
	vec2 ndcPos = 2.0 * uv - 1.0;
	return vec3(- ndcPos * viewDepth / vec2(projectionMatrix[0][0], projectionMatrix[1][1] ) , viewDepth);
}

/** Estimate the screen space ambient occlusion in the scene. */
void main(){
	
	vec3 n = normalize(2.0 * texture(normalTexture,In.uv).rgb - 1.0);
	
	// If normal is null, this is the background, no AO
	// (avoid reading the albedo texture just for the id.)
	if(length(n) < 0.1){
		fragColor = 1.0;
		return;
	}
	
	// Read the random local shift, uvs based on pixel coordinates (wrapping enabled).
	vec3 randomOrientation = texture(noiseTexture, gl_FragCoord.xy/5.0).rgb;
	
	// Create tangent space to view space matrix by computing tangent and bitangent.
	vec3 t = normalize(randomOrientation - n * dot(randomOrientation, n));
	vec3 b = normalize(cross(n, t));
	mat3 tbn = mat3(t, b, n);

	// Compute view space position.
	vec3 position = positionFromUV(In.uv);
	
	// Occlusion accumulation.
	float occlusion = 0.0;
	for(int i = 0; i < 24; ++i){
		// View space position of the sample.
		vec3 randomSample = position + RADIUS * tbn * samples[i];
		// Project view space point to clip space then NDC space.
		vec4 sampleClipSpace = projectionMatrix * vec4(randomSample, 1.0);
		vec2 sampleUV = (sampleClipSpace.xy / sampleClipSpace.w) * 0.5 + 0.5;
		// Read scene depth at the corresponding UV.
		float sampleDepth = linearizeDepth(texture(depthTexture, sampleUV).r);
		// Check : if the depth are too different, don't take result into account.
		float isValid = abs(position.z - sampleDepth) < RADIUS ? 1.0 : 0.0;
		// If the initial sample is further away from the camera than the surface, it is below the surface, occlusion is increased.
		occlusion += (sampleDepth >= randomSample.z  ? isValid : 0.0);
	}
	
	// Normalize and  reverse occlusion.
	occlusion = 1.0 - (occlusion/24.0);
	fragColor = pow(occlusion,2.0);
}
