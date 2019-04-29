#version 330 core

// Input vertex data, different every time this shader is executed
layout(location = 0) in vec3 vertexPosition_modelSpace; 

layout(location = 1) in vec3 vertexColor;

// Output data, for each fragment
out vec3 fragmentColor;

// Uniform that gets populated from C++ code
uniform mat4 MVP;

void main()
{
	// Outputs position transformed by the given MVP matrix
	gl_Position = MVP * vec4(vertexPosition_modelSpace, 1);

	fragmentColor = vertexColor;
}
