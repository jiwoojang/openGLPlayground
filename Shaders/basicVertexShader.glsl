#version 330 core

// Input vertex data, different every time this shader is executed
layout(location = 0) in vec3 vertexPosition_modelSpace; 

// Uniform that gets populated from C++ code
uniform mat4 MVP;

void main()
{
	// Outputs position transformed by the given MVP matrix
	gl_Position = MVP * vec4(vertexPosition_modelSpace, 1);
}
