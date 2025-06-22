#version 450

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 outColor;

layout(push_constant) uniform PushConstant {
	mat4 projection;
	mat4 model;
} pushConstants;

void main()
{
	gl_Position = pushConstants.projection * pushConstants.model * vec4(inPosition, 0.0, 1.0);
	outColor = inColor;
}