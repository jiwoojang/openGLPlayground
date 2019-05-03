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

#include <chrono>	// For high_resolution_clock
#include <cmath>	// For fmod
#include <algorithm>	// For clamp

using namespace glm;

// In degrees per second
const GLfloat ROTATION_SPEED = 1.0f;
const GLfloat COLOR_SPEED = 0.5f;

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
	glEnable(GL_CULL_FACE);

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

	// Data for a cube
	static const GLfloat gVertexBufferData[] =
	{
		-1.0f,-1.0f,-1.0f, // triangle 1 : begin
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f, // triangle 1 : end
		1.0f, 1.0f,-1.0f, // triangle 2 : begin
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f, // triangle 2 : end
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f
	};

	// One color for each vertex. They were generated randomly.
	static const GLfloat gColorBufferData[] = {
		0.583f,  0.771f,  0.014f,
		0.609f,  0.115f,  0.436f,
		0.327f,  0.483f,  0.844f,
		0.822f,  0.569f,  0.201f,
		0.435f,  0.602f,  0.223f,
		0.310f,  0.747f,  0.185f,
		0.597f,  0.770f,  0.761f,
		0.559f,  0.436f,  0.730f,
		0.359f,  0.583f,  0.152f,
		0.483f,  0.596f,  0.789f,
		0.559f,  0.861f,  0.639f,
		0.195f,  0.548f,  0.859f,
		0.014f,  0.184f,  0.576f,
		0.771f,  0.328f,  0.970f,
		0.406f,  0.615f,  0.116f,
		0.676f,  0.977f,  0.133f,
		0.971f,  0.572f,  0.833f,
		0.140f,  0.616f,  0.489f,
		0.997f,  0.513f,  0.064f,
		0.945f,  0.719f,  0.592f,
		0.543f,  0.021f,  0.978f,
		0.279f,  0.317f,  0.505f,
		0.167f,  0.620f,  0.077f,
		0.347f,  0.857f,  0.137f,
		0.055f,  0.953f,  0.042f,
		0.714f,  0.505f,  0.345f,
		0.783f,  0.290f,  0.734f,
		0.722f,  0.645f,  0.174f,
		0.302f,  0.455f,  0.848f,
		0.225f,  0.587f,  0.040f,
		0.517f,  0.713f,  0.338f,
		0.053f,  0.959f,  0.120f,
		0.393f,  0.621f,  0.362f,
		0.673f,  0.211f,  0.457f,
		0.820f,  0.883f,  0.371f,
		0.982f,  0.099f,  0.879f
	};

	// Two UV coordinatesfor each vertex. They were created with Blender.
	static const GLfloat gUVBufferData[] = {
		0.000059f, 0.000004f,
		0.000103f, 0.336048f,
		0.335973f, 0.335903f,
		1.000023f, 0.000013f,
		0.667979f, 0.335851f,
		0.999958f, 0.336064f,
		0.667979f, 0.335851f,
		0.336024f, 0.671877f,
		0.667969f, 0.671889f,
		1.000023f, 0.000013f,
		0.668104f, 0.000013f,
		0.667979f, 0.335851f,
		0.000059f, 0.000004f,
		0.335973f, 0.335903f,
		0.336098f, 0.000071f,
		0.667979f, 0.335851f,
		0.335973f, 0.335903f,
		0.336024f, 0.671877f,
		1.000004f, 0.671847f,
		0.999958f, 0.336064f,
		0.667979f, 0.335851f,
		0.668104f, 0.000013f,
		0.335973f, 0.335903f,
		0.667979f, 0.335851f,
		0.335973f, 0.335903f,
		0.668104f, 0.000013f,
		0.336098f, 0.000071f,
		0.000103f, 0.336048f,
		0.000004f, 0.671870f,
		0.336024f, 0.671877f,
		0.000103f, 0.336048f,
		0.336024f, 0.671877f,
		0.335973f, 0.335903f,
		0.667969f, 0.671889f,
		1.000004f, 0.671847f,
		0.667979f, 0.335851f
	};

	// Load Texture
	GLuint Texture = loadDDS("uvtemplate.dds");

	// ID for buffers
	GLuint vertexBuffer;
	GLuint colorBuffer;
	GLuint UVBuffer;

	// Generates a buffer and puts resulting ID in buffer ID's
	// NOTE, THE ORDER IN THE FOLLOWING FUNCTION CALLS MATTERS! GROUP THEM TOGETHER
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gVertexBufferData), gVertexBufferData, GL_STATIC_DRAW);

	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gColorBufferData), gColorBufferData, GL_STATIC_DRAW);
	
	glGenBuffers(1, &UVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, UVBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gUVBufferData), gUVBufferData, GL_STATIC_DRAW);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	GLuint programID = LoadShaders("basicVertexShader.glsl", "basicFragmentShader.glsl");

	// Model matrix set up
	// Rotate about Z axis
	vec3 modelRotationAxis(0, 1.0f, 0);
	mat4 modelMatrix = mat4(1.0f);
	GLfloat angle = 0.0f;

	// Get the "uniform" that is named MVP in our shader
	GLuint matrixID = glGetUniformLocation(programID, "MVP");

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

		// Attribute for color buffer
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);

		// Increment color
		colorVal += COLOR_SPEED * deltaTime;

		static GLfloat g_color_buffer_data[12 * 3 * 3];
		for (int v = 0; v < 12 * 3; v++) {
			g_color_buffer_data[3 * v + 0] = fmod(colorVal, 1.0f);
			g_color_buffer_data[3 * v + 1] = 1.0 - fmod(colorVal, 1.0f);
			g_color_buffer_data[3 * v + 2] = fmod(colorVal, 1.0f);
		}

		glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

		glVertexAttribPointer
		(
			1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Attribute for UV buffer
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, UVBuffer);
		glVertexAttribPointer
		(
			2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			2,                  // size 2, because 2D coordinates
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		//glBufferData(GL_ARRAY_BUFFER, sizeof(gUVBufferData), gUVBufferData, GL_STATIC_DRAW);

		glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles -> 6 squares

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Get time between this loop and the last one
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = end - begin;

		auto durationInMS = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
		auto durationInS = durationInMS / 1000000.0f;

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

