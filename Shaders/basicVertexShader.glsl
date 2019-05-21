#version 330 core

// Input vertex data, different every time this shader is executed
layout(location = 0) in vec3 vertexPosition_modelSpace; 

layout(location = 1) in vec2 vertexUV;

layout(location = 2) in vec3 vertexNormal_modelSpace;

// Output data, for each fragment
out vec2 UV;
out vec3 position_worldSpace;
out vec3 eyeDirection_cameraSpace;
out vec3 lightDirection_cameraSpace;
out vec3 normal_cameraSpace;

// Stays constant for entire mesh
uniform mat4 MVP;
uniform mat4 M;
uniform mat4 V;
uniform vec3 lightPosition_worldSpace;

void main()
{
	// Outputs position transformed by the given MVP matrix
	gl_Position = MVP * vec4(vertexPosition_modelSpace, 1);

	// Position of vertex in worldspace
	position_worldSpace = (M * vec4(vertexPosition_modelSpace, 1)).xyz;

	// Vector from vertex to camera center, with camera center at origin in camera space
	vec3 vertexPosition_cameraSpace = ( V * M * vec4(vertexPosition_modelSpace,1)).xyz;
	eyeDirection_cameraSpace = vec3(0,0,0) - vertexPosition_cameraSpace;

	// Vector from vertex to light. Lights have identity M matrix
	vec3 lightPosition_cameraSpace = ( V * vec4(lightPosition_worldSpace,1)).xyz;
	lightDirection_cameraSpace = lightPosition_cameraSpace + eyeDirection_cameraSpace;

	// Vertex Normal in camera space.
	// Only correct if Model Matrix does not scale the model ! Use its inverse transpose if not.
	normal_cameraSpace = (V * M * vec4(vertexNormal_modelSpace, 0)).xyz;

	// UV of the vertex
    UV = vertexUV;
}
