#version 330 core

// Interpolated values from vertex shader
in vec2 UV;
in vec3 position_worldSpace;
in vec3 eyeDirection_cameraSpace;
in vec3 lightDirection_cameraSpace;
in vec3 normal_cameraSpace;

// Output data
out vec3 color;

// Values that stay constant for the whole mesh
uniform sampler2D	textureSampler;
uniform vec3		lightColor;
uniform float		lightStrength;
uniform vec3		lightPosition_worldSpace;

void main()
{
	vec3 n = normalize(normal_cameraSpace);
	vec3 l = normalize(lightDirection_cameraSpace);
	vec3 e = normalize(eyeDirection_cameraSpace);

	vec3 materialDiffuseColor =		texture(textureSampler, UV).rgb;
	vec3 materialAmbientColor =		vec3(0.1,0.1,0.1) * materialDiffuseColor;
	vec3 materialSpecularColor =	vec3(0.3,0.3,0.3);

	//--------------------------------------------
	// Diffuse Reflection Calcs
	//--------------------------------------------

	// Cosine of the angle between the normal and the light direction, clamped above 0
	float cosTheta = clamp( dot( n,l ), 0,1 );

	// Distance to the light
	float distance = length( lightPosition_worldSpace - position_worldSpace );

	//--------------------------------------------
	// Specular Reflection Calcs
	//--------------------------------------------

	vec3 r = reflect(-l, n);

	// Cosine of the angle between direction the camera faces and the direction of reflection
	float cosAlpha = clamp( dot( e,r ), 0,1 );

	//--------------------------------------------
	// Final Color Calc
	//--------------------------------------------

	color = 

	// Ambient lighting
	materialAmbientColor +
	
	// Diffuse lighting from object color
	// Light intensity is inversely proprtional to square of distance
	materialDiffuseColor * lightColor * lightStrength * cosTheta / (distance * distance) +

	// Specilar lighting from reflection intensity
	materialSpecularColor * lightColor * lightStrength * pow(cosAlpha,5) / (distance*distance);
}