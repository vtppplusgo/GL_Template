#version 330

// Input: UV coordinates
in INTERFACE {
	vec2 uv;
} In ; ///< vec2 uv;

layout(binding = 0) uniform sampler2D screenTexture; ///< Image to blur.

layout(location = 0) out vec2 fragColor; ///< Color.

/** Perform a 5x5 box blur on the input image. */
void main(){
	
	// We have to unroll the box blur loop manually.
	
	vec2 color = textureOffset(screenTexture, In.uv, ivec2(-2,-2)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(-2,-1)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(-2,0)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(-2,1)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(-2,2)).rg;
	
	color += textureOffset(screenTexture, In.uv, ivec2(-1,-2)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(-1,-1)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(-1,0)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(-1,1)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(-1,2)).rg;
	
	color += textureOffset(screenTexture, In.uv, ivec2(0,-2)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(0,-1)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(0,0)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(0,1)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(0,2)).rg;
	
	color += textureOffset(screenTexture, In.uv, ivec2(1,-2)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(1,-1)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(1,0)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(1,1)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(1,2)).rg;
	
	color += textureOffset(screenTexture, In.uv, ivec2(2,-2)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(2,-1)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(2,0)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(2,1)).rg;
	color += textureOffset(screenTexture, In.uv, ivec2(2,2)).rg;
	
	fragColor = color / 25.0;
}
