#version 450
#extension GL_EXT_scalar_block_layout: require
// input from vertex shader
layout(location = 0) in vec2 fragUV;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 2) uniform sampler _sampler;

layout(std430, set = 1, binding = 0) uniform UBO2
{
	vec2 extent;
} fx;

layout(set = 2, binding = 0) uniform texture2D attachment;

void main()
{
	vec2 resolution = vec2(1920, 1080);
	vec2 uv = fragUV; //gl_FragCoord.xy / resolution;
	outColor = texture(sampler2D(attachment, _sampler), uv);
	gl_FragDepth = 0.99999;
}