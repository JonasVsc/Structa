#version 450
#extension GL_EXT_buffer_reference : require

layout(location = 0) out vec3 outColor;

struct Vertex {
	vec2 position;
	vec3 color;
}; 

layout(buffer_reference, std430) readonly buffer VertexBuffer{ 
	Vertex vertices[];
};

layout( push_constant ) uniform constants
{	
	VertexBuffer vertexBuffer;
} PushConstants;

void main() 
{	
	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	//output data
	gl_Position = vec4(v.position, 0.0, 1.0);
	outColor = v.color.xyz;
}