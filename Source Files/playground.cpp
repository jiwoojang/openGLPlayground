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
using namespace glm;

// In degrees per second
const GLfloat ROTATION_SPEED = 1.0f;
const GLfloat COLOR_SPEED = 0.5f;

// Time between most two frames in seconds
GLfloat deltaTime = 0.0f;

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

	// ID for buffers
	GLuint vertexBuffer;
	GLuint colorBuffer;

	// Generates a buffer and puts resulting ID in buffer ID's
	// NOTE, THE ORDER IN THE FOLLOWING FUNCTION CALLS MATTERS! GROUP THEM TOGETHER
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gVertexBufferData), gVertexBufferData, GL_STATIC_DRAW);

	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gColorBufferData), gColorBufferData, GL_STATIC_DRAW);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	GLuint programID = LoadShaders("basicVertexShader.glsl", "basicFragmentShader.glsl");

	// 85 degree FOV, 16 x 9 aspect ratio
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(85.0f), 16.0f / 9.0f, 0.1f, 100.0f);

	// Camera is at (4,3,3), looking at (0,0,0) and the world up vector is (0,1,0)
	glm::mat4 viewMatrix = glm::lookAt(glm::vec3(4, 3, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	// Model matrix set up
	// Rotate about Z axis
	glm::vec3 modelRotationAxis(0, 0, 1.0f);
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	GLfloat angle = 0.0f;

	// Note the order of multiplication here!!
	glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;

	// Get the "uniform" that is named MVP in our shader
	GLuint matrixID = glGetUniformLocation(programID, "MVP");

	// First time stamp
	auto begin = std::chrono::high_resolution_clock::now();

	GLfloat colorVal = 0.0f;

	do{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use the shaders we set up earlier
		glUseProgram(programID);

		// Create a rotation matrix about the z axis
		glm::mat4 rotationMatrix = glm::rotate((ROTATION_SPEED * deltaTime), modelRotationAxis);

		// Apply rotation matrix to model matrix
		modelMatrix *= rotationMatrix;

		// Apply model matrix to MVP matrix
		glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;

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

		glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles -> 6 squares

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

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

