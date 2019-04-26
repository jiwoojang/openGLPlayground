#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>	// For matrix transformation functions
#include <glm/gtx/transform.hpp>	// For rotation matrix
#include <common/shader.hpp>	// For loading shaders

#include <chrono>	// For high_resolution_clock;
using namespace glm;

// In degrees per second
const GLfloat ROTATION_SPEED = 10.0f;

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

	static const GLfloat gVertexBufferData[] =
	{
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
	};

	// ID for vertex buffer
	GLuint vertexBuffer;

	// Generates a buffer and puts resulting ID in vertexBuffer
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gVertexBufferData), gVertexBufferData, GL_STATIC_DRAW);

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

		glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
		glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		// Get time between this loop and the last one
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = end - begin;

		auto durationInMS = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
		auto durationInS = durationInMS / 1000000.0f;

		deltaTime = durationInS;

		printf("deltaTime is: %f\n", deltaTime);

		// Update last time counted
		begin = end;

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

