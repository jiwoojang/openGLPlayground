#version 330 core

// Interpolated values from vertex shader
in vec2 UV;

// Output data
out vec3 color;

// Values that stay constant for the whole mesh
uniform sampler2D textureSampler;

void main()
{
	color = texture(textureSampler, UV).rgb;
}