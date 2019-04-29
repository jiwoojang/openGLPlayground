#version 330 core

// Output data
out vec3 color;

// To recieve from the vertex shader 
in vec3 fragmentColor; 

void main()
{
	color = fragmentColor;
}