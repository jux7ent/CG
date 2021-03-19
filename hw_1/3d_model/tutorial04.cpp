// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>

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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Tutorial 04 - Colored Cube", NULL, NULL);
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

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	glm::mat4 View       = glm::lookAt(
								glm::vec3(4,3,-3), // Camera is at (4,3,-3), in World Space
								glm::vec3(0,0,0), // and looks at the origin
								glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
						   );
	// Model matrix : an identity matrix (model will be at the origin)
	glm::vec3 rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::mat4 Model      = glm::mat4(1.0f);
	Model = glm::rotate(Model, -90 * (3.14f / 180.0f), rotationAxis);

	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around

	// Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
	// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
	static const GLfloat g_vertex_buffer_data[] = { 
		0.00, 0.53, 0.00,
		0.29, 0.24, -0.38,
		-0.27, 0.24, -0.39,
		0.00, 0.53, 0.00,
		0.45, 0.24, 0.16,
		0.29, 0.24, -0.38,
		0.00, 0.53, 0.00,
		-0.01, 0.24, 0.48,
		0.45, 0.24, 0.16,
		0.00, 0.53, 0.00,
		-0.46, 0.24, 0.14,
		-0.01, 0.24, 0.48,
		0.00, 0.53, 0.00,
		-0.27, 0.24, -0.39,
		-0.46, 0.24, 0.14,
		-0.27, 0.24, -0.39,
		0.01, -0.24, -0.48,
		-0.45, -0.24, -0.16,
		0.29, 0.24, -0.38,
		0.46, -0.24, -0.14,
		0.01, -0.24, -0.48,
		0.45, 0.24, 0.16,
		0.27, -0.24, 0.39,
		0.46, -0.24, -0.14,
		-0.01, 0.24, 0.48,
		-0.29, -0.24, 0.38,
		0.27, -0.24, 0.39,
		-0.46, 0.24, 0.14,
		-0.45, -0.24, -0.16,
		-0.29, -0.24, 0.38,
		0.01, -0.24, -0.48,
		-0.27, 0.24, -0.39,
		0.29, 0.24, -0.38,
		0.46, -0.24, -0.14,
		0.29, 0.24, -0.38,
		0.45, 0.24, 0.16,
		0.27, -0.24, 0.39,
		0.45, 0.24, 0.16,
		-0.01, 0.24, 0.48,
		-0.29, -0.24, 0.38,
		-0.01, 0.24, 0.48,
		-0.46, 0.24, 0.14,
		-0.45, -0.24, -0.16,
		-0.46, 0.24, 0.14,
		-0.27, 0.24, -0.39,
		0.00, -0.53, 0.00,
		0.01, -0.24, -0.48,
		0.46, -0.24, -0.14,
		0.00, -0.53, 0.00,
		0.46, -0.24, -0.14,
		0.27, -0.24, 0.39,
		0.00, -0.53, 0.00,
		0.27, -0.24, 0.39,
		-0.29, -0.24, 0.38,
		0.00, -0.53, 0.00,
		-0.29, -0.24, 0.38,
		-0.45, -0.24, -0.16,
		0.00, -0.53, 0.00,
		-0.45, -0.24, -0.16,
		0.01, -0.24, -0.48,
	};

	// One color for each vertex. They were generated randomly.
	static const GLfloat g_color_buffer_data[] = { 
		0.03, 0.67, 0.76,
		0.07, 0.38, 0.37,
		0.50, 0.10, 0.10,
		0.91, 0.52, 0.07,
		0.03, 0.24, 0.24,
		1.00, 0.50, 0.80,
		0.71, 0.93, 0.83,
		0.17, 0.46, 0.78,
		0.54, 0.96, 0.20,
		0.72, 0.96, 0.45,
		0.73, 0.11, 0.68,
		0.62, 0.68, 0.37,
		0.80, 0.16, 0.25,
		0.14, 0.02, 0.57,
		0.47, 0.86, 0.14,
		0.59, 0.82, 0.21,
		0.73, 1.00, 0.37,
		0.60, 0.04, 0.56,
		0.70, 0.30, 0.75,
		0.37, 0.20, 0.65,
		0.10, 0.45, 0.56,
		0.85, 0.18, 0.44,
		0.04, 0.36, 0.72,
		0.18, 0.04, 0.39,
		0.63, 0.39, 0.04,
		0.89, 0.87, 0.75,
		0.27, 0.61, 0.17,
		0.79, 0.70, 0.72,
		0.58, 0.46, 0.58,
		0.83, 0.81, 0.64,
		0.62, 0.31, 0.95,
		0.47, 0.23, 0.95,
		0.02, 0.20, 0.55,
		0.04, 0.04, 0.98,
		0.88, 0.55, 0.98,
		0.24, 0.85, 0.12,
		0.29, 0.25, 0.80,
		0.59, 0.28, 0.61,
		0.81, 0.32, 0.43,
		0.69, 0.47, 0.69,
		0.02, 0.51, 0.22,
		0.66, 0.80, 0.76,
		0.18, 0.92, 0.75,
		0.74, 0.76, 0.59,
		0.29, 0.64, 0.34,
		0.91, 0.40, 0.54,
		0.11, 0.27, 0.87,
		0.93, 0.82, 0.32,
		0.42, 0.88, 0.59,
		0.39, 0.15, 0.89,
		0.97, 0.67, 0.14,
		0.54, 0.90, 0.16,
		0.84, 0.41, 0.37,
		0.55, 0.81, 0.57,
		0.88, 0.79, 0.20,
		0.44, 0.80, 0.74,
		0.80, 0.91, 0.95,
		0.36, 0.22, 0.50,
		0.74, 0.69, 0.79,
		0.30, 0.72, 0.33,
	};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

	const float radius = 3.0f;

	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float camera_x = sin(glfwGetTime()) * radius;
		float camera_y = sin(glfwGetTime()) * radius;
		float camera_z = cos(glfwGetTime()) * radius;

		View = glm::lookAt(
			glm::vec3(camera_x, camera_y, camera_z),
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0) // Head is up (set to 0,-1,0 to look upside-down)
		);

		MVP = Projection * View * Model;

		// Use our shader
		glUseProgram(programID);

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 64); // 64 vertices

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

