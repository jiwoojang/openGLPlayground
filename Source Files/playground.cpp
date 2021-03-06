#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>	// For matrix transformation functions
#include <glm/gtx/transform.hpp>	// For rotation matrix
#include <common/shader.hpp>	// For loading shaders
#include <common/objBasicLoader.hpp>	// For loading obj files
#include <common/vboindexer.hpp>	// For VBO indexing

#include <chrono>	// For high_resolution_clock
#include <cmath>	// For fmod
#include <algorithm>	// For clamp
#include <vector>

using namespace glm;

// In degrees per second
const GLfloat ROTATION_SPEED =	1.0f;
const GLfloat COLOR_SPEED =		0.5f;

// In watts
const GLfloat LIGHT_INTENSITY = 50.0f;
// In RGB
const vec3 LIGHT_COLOR =		vec3(1, 1, 1);

// Time between most two frames in seconds
GLfloat deltaTime = 0.0f;

// BMP loading function  
GLuint loadBMP_custom(const char * imagepath);
GLuint loadDDS(const char * imagepath);

// Interfacing function
void computeMatriciesFromInputs();
mat4 getProjectionMatrix();
mat4 getViewMatrix();

// Defaults for interfacing functions
vec3 position = vec3(0, 0, 5);
mat4 projectionMatrix;
mat4 viewMatrix;

float horizontalAngle = M_PI;
float verticalAngle = 0;

float FOV = 45.0f;

int windowWidth = 0;
int windowHeight = 0;

const float MOVE_SPEED = 3.0f;
const float MOUSE_SENS = 0.05f;
const float ALPHA = 0.6f;

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Playground", NULL, NULL);

	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	// Scroll callbacks for zoom
	glfwSetScrollCallback(window, scrollCallback);

	// Enable backface culling
	//glEnable(GL_CULL_FACE);

	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	
	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Black blue background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Create a vertex array object
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Load from obj file
	std::vector <vec3> vertices; 
	std::vector <vec2> uvs;
	std::vector <vec3> normals;
	
	if (!loadOBJ("suzanne.obj", vertices, uvs, normals))
	{
		printf("Failed to load OBJ\n");
	}

	// Fill VBO index buffer
	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;

	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	// Load Texture
	GLuint texture = loadDDS("suzanneuvmap.dds");

	// ID for buffers
	GLuint vertexBuffer;
	GLuint UVBuffer;
	GLuint normalBuffer;
	GLuint elementBuffer;

	// Generates a buffer and puts resulting ID in buffer ID's
	// NOTE, THE ORDER IN THE FOLLOWING FUNCTION CALLS MATTERS! GROUP THEM TOGETHER
	// We use the indexed buffers now
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(vec3), &indexed_vertices[0], GL_STATIC_DRAW);
	
	glGenBuffers(1, &UVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, UVBuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(vec3), &indexed_uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Element buffer for VBO indexing
	glGenBuffers(1, &elementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);

	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Load up shaders
	GLuint programID = LoadShaders("basicVertexShader.glsl", "basicFragmentShader.glsl");

	// Model matrix set up
	// Rotate about Z axis
	vec3 modelRotationAxis(0, 1.0f, 0);
	mat4 modelMatrix = mat4(1.0f);
	GLfloat angle = 0.0f;

	// Get the "uniforms" that is named according to the values in our shader
	GLuint matrixID =	glGetUniformLocation(programID, "MVP");
	GLuint mID =		glGetUniformLocation(programID, "M");
	GLuint vID =		glGetUniformLocation(programID, "V");
	GLuint textureID =	glGetUniformLocation(programID, "myTextureSampler");
	GLuint lightID =	glGetUniformLocation(programID, "lightPosition_worldSpace");
	GLuint colorID =	glGetUniformLocation(programID, "lightColor");
	GLuint strengthID =	glGetUniformLocation(programID, "lightStrength");
	GLuint alphaID =	glGetUniformLocation(programID, "alpha");

	// First time stamp
	auto begin = std::chrono::high_resolution_clock::now();

	GLfloat colorVal = 0.0f;

	do{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use the shaders we set up earlier
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatriciesFromInputs();

		// Create a rotation matrix about the z axis
		mat4 rotationMatrix = rotate((ROTATION_SPEED * deltaTime), modelRotationAxis);

		// Apply rotation matrix to model matrix
		modelMatrix *= rotationMatrix;

		// Apply model matrix to MVP matrix
		mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;

		// Send the matrix to the shader
		glUniformMatrix4fv(matrixID, 1, GL_FALSE, &mvpMatrix[0][0]);
		glUniformMatrix4fv(mID, 1, GL_FALSE, &modelMatrix[0][0]);
		glUniformMatrix4fv(vID, 1, GL_FALSE, &viewMatrix[0][0]);

		// Set up light
		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(lightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(colorID, LIGHT_COLOR.x, LIGHT_COLOR.y, LIGHT_COLOR.z);
		glUniform1f(strengthID, LIGHT_INTENSITY);

		// Set up alpha channel
		glUniform1f(alphaID, ALPHA);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(textureID, 0);

		// Draw triangle
		// Attribute for vertex buffer
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer
		(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Attribute for UV buffer
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, UVBuffer);
		glVertexAttribPointer
		(
			1,                  // attribute 2
			2,                  // size 2, because 2D coordinates
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glVertexAttribPointer(
			2,                                // attribute 3
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,      // mode
			indices.size(),    // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0           // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Get time between this loop and the last one
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = end - begin;

		auto durationInMS = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
		auto durationInS = durationInMS / 1000000.0f;

		printf("%f ms per frame\n", durationInMS / 1000.0f);
		
		deltaTime = durationInS;

		// Update last time counted
		begin = end;

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

// Ghetto function, made just because I can
// This will be replaced by a library function later
GLuint loadBMP_custom(const char * imagepath)
{
	// Data read from BMP file header
	unsigned char header[54];	// BMP files start with a 54 byte header
	unsigned int dataPos;		// Position in the file where the actual data begins
	unsigned int width, height;
	unsigned int imageSize;		// = width*height*3 because one for each RGB
	unsigned char *data;		// Actual RGB data

	FILE *file = fopen(imagepath, "rb");

	// Validate file opening
	if (!file)
	{
		printf("Image could not be opened! \n");
		return 0;
	}

	// Validate header against BMP format
	if (fread(header, 1, 54, file) != 54)
	{
		printf("Header for this BMP file is not correct, or it is not a BMP \n");
		return 0;
	}

	// The first two bytes of a BMP are "BM"
	if (header[0] != 'B' || header[1] != 'M')
	{
		printf("Byte check for this BMP file is not correct, or it is not a BMP \n");
		return 0;
	}

	// Read ints from the byte array
	dataPos		= *(int*)&(header[0x0A]);
	imageSize	= *(int*)&(header[0x22]);
	width		= *(int*)&(header[0x12]);
	height		= *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)    imageSize = width * height * 3;
	if (dataPos == 0)      dataPos = 54;	// Bypass the 54 byte header

	// Create a new buffer for the data
	data = new unsigned char[imageSize];

	// Read the data from the file into the buffer
	fread(data, 1, imageSize, file);

	// Close the file, we are done with it
	fclose(file);

	// ID for textures
	GLuint textureID;

	glGenTextures(1, &textureID);

	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Pass the image into OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// When MAGnifying the image (no bigger mipmap available), use LINEAR filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// When MINifying the image, use a LINEAR blend of two mipmaps, each filtered LINEARLY too
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// Generate mipmaps, by the way.
	glGenerateMipmap(GL_TEXTURE_2D);

	return 1;
}

#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

GLuint loadDDS(const char * imagepath)
{
	// 124 byte header this time for DDS files
	unsigned char header[124];

	FILE *file;

	file = fopen(imagepath, "rb");

	// Validate file opening
	if (file == NULL)
	{
		printf("Could not load file\n");
		return 0;
	}

	// Validate the header
	char filecode[4];
	fread(filecode, 1, 4, file);

	// Header for DDS files reads 'DDS'
	if (strncmp(filecode, "DDS ", 4) != 0)
	{
		fclose(file);
		return 0;
	}

	// Get the surface desc
	fread(&header, 124, 1, file);

	unsigned int height			= *(unsigned int*)&(header[8]);
	unsigned int width			= *(unsigned int*)&(header[12]);
	unsigned int linearSize		= *(unsigned int*)&(header[16]);
	unsigned int mipMapCount	= *(unsigned int*)&(header[24]);
	unsigned int fourCC			= *(unsigned int*)&(header[80]);

	// Now read all the actual data, and the mipmaps successively
	unsigned char *buffer; 
	unsigned int bufsize; 

	// If there are multiple mipmaps, increase buffer size
	bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;

	buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
	fread(buffer, 1, bufsize, file);

	fclose(file);

	// Convert flag into value parseable by OpenGL
	unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
	unsigned int format;
	switch (fourCC)
	{
	case FOURCC_DXT1:
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case FOURCC_DXT3:
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case FOURCC_DXT5:
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	default:
		free(buffer);
		return 0;
	}

	GLuint textureID;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);

	// Alocate for the mipmaps
	unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
	unsigned int offset = 0;

	// Fill in the mipmaps
	for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
	{
		// Fill in data
		unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, buffer + offset);
		
		// increment offset and change width/height
		offset += size;
		width /= 2;
		height /= 2;

		// Deal with Non-Power-Of-Two textures
		if (width < 1) 
			width = 1;

		if (height < 1)
			height = 1;
	}

	free(buffer);
	return textureID;
}

void computeMatriciesFromInputs()
{
	// Get mouse position
	double xPos, yPos;
	glfwGetCursorPos(window, &xPos, &yPos);

	// Reset mouse position at center screen for next frame
	glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);

	// Compute Angles
	horizontalAngle	+= MOUSE_SENS * deltaTime * float(windowWidth / 2 - xPos);
	verticalAngle	+= MOUSE_SENS * deltaTime * float(windowHeight / 2 - yPos);

	// I know this casting is gross
	verticalAngle = clamp((double)verticalAngle, -M_PI_2, M_PI_2);

	// Generate Spherical coordinate direciton based on angles 
	vec3 forward(cos(verticalAngle) * sin(horizontalAngle), sin(verticalAngle), cos(verticalAngle) * cos(horizontalAngle));

	// Right vector is always horizontal
	vec3 right(sin(horizontalAngle - M_PI_2), 0, cos(horizontalAngle - M_PI_2));

	// Up vector is cross of forward and right
	vec3 up = cross(right, forward);

	// Move forward
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		position += forward * deltaTime * MOVE_SPEED;
	}

	// Move backward
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		position -= forward * deltaTime * MOVE_SPEED;
	}

	// Strafe right
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position += right * deltaTime * MOVE_SPEED;
	}

	// Strafe left
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position -= right * deltaTime * MOVE_SPEED;
	}

	// Rise Up
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		position += up * deltaTime * MOVE_SPEED;
	}

	// Sink Down
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		position -= up * deltaTime * MOVE_SPEED;
	}

	projectionMatrix = perspective(radians(FOV), 4.0f / 3.0f, 0.1f, 100.0f);
	viewMatrix = lookAt(position, position + forward, up);
}

mat4 getProjectionMatrix()
{
	return projectionMatrix;
}

mat4 getViewMatrix()
{
	return viewMatrix;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	FOV -= 5 * yoffset;
}

