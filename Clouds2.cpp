// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <shader.hpp>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define M 2147483647
#define A 16807
#define Q ( M / A )
#define R ( M % A )

struct vertex {
	GLfloat x, y, z;
	GLfloat r, g, b;
};

int seed;

/*
long niminus1 = 8l;
long myrand(long min, long max) {
	long range = max - min;
	niminus1 = (niminus1 * 5l + 12345l);
	return niminus1 % max;
}

long jsw_lcg(long max) {
	niminus1 = ( 2l * niminus1 + 3l ) % 10l;
	return niminus1 % max;
}
 */

int jsw_rand(int min, int max) {
	seed = A * ( seed % Q ) - R * ( seed / Q );
	if ( seed <= 0 )
		seed += M;
	//return seed % (max + 1);
	return min + ( seed % ( (max + 1) - min ) );
}

int main() {

	/*printf("jsw_lcg():\n");
	for (int i = 0; i < 20; i++) {
		printf("%ld\n", jsw_lcg(15));
	}
	printf("\n");*/

	/*printf("myrand():\n");
	for (int i = 0; i < 20; i++) {
		printf("%ld\n", myrand(0, 10));
	}*/

	// Initialise GLFW
	if(!glfwInit()) {
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Clouds", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "SimpleTransform.vertexshader", "SingleColor.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.0f, 100.0f);
	// Or, for an ortho camera :
	//glm::mat4 Projection = glm::ortho(-1.0f, 50.0f, -1.0f,50.0f,0.0f,100.0f); // In world coordinates

	// Camera matrix
	glm::mat4 View       = glm::lookAt(
			glm::vec3(17500, 16700, 45000), // Camera is at (4,3,3), in World Space
			glm::vec3(17500, 16700, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model      = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

	const int numOfRandomPointsPerGraph = 7000;
	const int numOfGraphs = 4;
	const int numOfxyAxisPoints = 3 * numOfGraphs;
	const int totalNumOfNonAxisPoints = numOfRandomPointsPerGraph * numOfGraphs;
	const int totalNumberOfPoints = totalNumOfNonAxisPoints + numOfxyAxisPoints;

	vertex g_vertex_buffer_data[totalNumberOfPoints];

	int g_vertex_buffer_data2_index = numOfRandomPointsPerGraph;
	vertex* g_vertex_buffer_data2 = &g_vertex_buffer_data[g_vertex_buffer_data2_index];

	int g_vertex_buffer_data3_index = numOfRandomPointsPerGraph * 2;
	vertex* g_vertex_buffer_data3 = &g_vertex_buffer_data[g_vertex_buffer_data3_index];

	int g_vertex_buffer_data4_index = numOfRandomPointsPerGraph * 3;
	vertex* g_vertex_buffer_data4 = &g_vertex_buffer_data[g_vertex_buffer_data4_index];

	int xyAxis1_index = totalNumberOfPoints - 12;
	vertex* xyAxis1 = &g_vertex_buffer_data[xyAxis1_index];

	int xyAxis2_index = totalNumberOfPoints - 9;
	vertex* xyAxis2 = &g_vertex_buffer_data[xyAxis2_index];

	int xyAxis3_index = totalNumberOfPoints - 6;
	vertex* xyAxis3 = &g_vertex_buffer_data[xyAxis3_index];

	int xyAxis4_index = totalNumberOfPoints - 3;
	vertex* xyAxis4 = &g_vertex_buffer_data[xyAxis4_index];

	srand(time(NULL)); //initialize random seed
	seed = time(NULL) % INT_MAX;

	//data 1
	for (int i = 0; i < numOfRandomPointsPerGraph; i++) {
		g_vertex_buffer_data[i].x = jsw_rand(0, RAND_MAX);
		g_vertex_buffer_data[i].y = jsw_rand(0, RAND_MAX);
		g_vertex_buffer_data[i].z = 0.0f;
		g_vertex_buffer_data[i].r = jsw_rand(0, 101) / 100.0f;
		g_vertex_buffer_data[i].g = jsw_rand(0, 101) / 100.0f;
		g_vertex_buffer_data[i].b = jsw_rand(0, 101) / 100.0f;
	}

	xyAxis1[0].x = 0.0f;
	xyAxis1[0].y = RAND_MAX;
	xyAxis1[0].z = 0.0f;
	xyAxis1[0].r = 1.0f;
	xyAxis1[0].g = 1.0f;
	xyAxis1[0].b = 1.0f;

	xyAxis1[1].x = 0.0f;
	xyAxis1[1].y = 0.0f;
	xyAxis1[1].z = 0.0f;
	xyAxis1[1].r = 1.0f;
	xyAxis1[1].g = 1.0f;
	xyAxis1[1].b = 1.0f;

	xyAxis1[2].x = RAND_MAX;
	xyAxis1[2].y = 0.0f;
	xyAxis1[2].z = 0.0f;
	xyAxis1[2].r = 1.0f;
	xyAxis1[2].g = 1.0f;
	xyAxis1[2].b = 1.0f;

	//data 2
	for (int i = 0; i < numOfRandomPointsPerGraph; i++) {
		g_vertex_buffer_data2[i].x = rand();
		g_vertex_buffer_data2[i].y = rand();
		g_vertex_buffer_data2[i].z = 0.0f;
		g_vertex_buffer_data2[i].r = (rand() % 101) / 100.0f;
		g_vertex_buffer_data2[i].g = (rand() % 101) / 100.0f;
		g_vertex_buffer_data2[i].b = (rand() % 101) / 100.0f;
	}

	xyAxis2[0].x = 0.0f;
	xyAxis2[0].y = RAND_MAX;
	xyAxis2[0].z = 0.0f;
	xyAxis2[0].r = 1.0f;
	xyAxis2[0].g = 1.0f;
	xyAxis2[0].b = 1.0f;

	xyAxis2[1].x = 0.0f;
	xyAxis2[1].y = 0.0f;
	xyAxis2[1].z = 0.0f;
	xyAxis2[1].r = 1.0f;
	xyAxis2[1].g = 1.0f;
	xyAxis2[1].b = 1.0f;

	xyAxis2[2].x = RAND_MAX;
	xyAxis2[2].y = 0.0f;
	xyAxis2[2].z = 0.0f;
	xyAxis2[2].r = 1.0f;
	xyAxis2[2].g = 1.0f;
	xyAxis2[2].b = 1.0f;

	//data 3
	for (int i = 0; i < numOfRandomPointsPerGraph; i++) {
		g_vertex_buffer_data3[i].x = rand();
		g_vertex_buffer_data3[i].y = rand();
		g_vertex_buffer_data3[i].z = 0.0f;
		g_vertex_buffer_data3[i].r = (rand() % 101) / 100.0f;
		g_vertex_buffer_data3[i].g = (rand() % 101) / 100.0f;
		g_vertex_buffer_data3[i].b = (rand() % 101) / 100.0f;
	}

	xyAxis3[0].x = 0.0f;
	xyAxis3[0].y = RAND_MAX;
	xyAxis3[0].z = 0.0f;
	xyAxis3[0].r = 1.0f;
	xyAxis3[0].g = 1.0f;
	xyAxis3[0].b = 1.0f;

	xyAxis3[1].x = 0.0f;
	xyAxis3[1].y = 0.0f;
	xyAxis3[1].z = 0.0f;
	xyAxis3[1].r = 1.0f;
	xyAxis3[1].g = 1.0f;
	xyAxis3[1].b = 1.0f;

	xyAxis3[2].x = RAND_MAX;
	xyAxis3[2].y = 0.0f;
	xyAxis3[2].z = 0.0f;
	xyAxis3[2].r = 1.0f;
	xyAxis3[2].g = 1.0f;
	xyAxis3[2].b = 1.0f;

	//data 4
	for (int i = 0; i < numOfRandomPointsPerGraph; i++) {
		g_vertex_buffer_data4[i].x = rand();
		g_vertex_buffer_data4[i].y = rand();
		g_vertex_buffer_data4[i].z = 0.0f;
		g_vertex_buffer_data4[i].r = (rand() % 101) / 100.0f;
		g_vertex_buffer_data4[i].g = (rand() % 101) / 100.0f;
		g_vertex_buffer_data4[i].b = (rand() % 101) / 100.0f;
	}

	xyAxis4[0].x = 0.0f;
	xyAxis4[0].y = RAND_MAX;
	xyAxis4[0].z = 0.0f;
	xyAxis4[0].r = 1.0f;
	xyAxis4[0].g = 1.0f;
	xyAxis4[0].b = 1.0f;

	xyAxis4[1].x = 0.0f;
	xyAxis4[1].y = 0.0f;
	xyAxis4[1].z = 0.0f;
	xyAxis4[1].r = 1.0f;
	xyAxis4[1].g = 1.0f;
	xyAxis4[1].b = 1.0f;

	xyAxis4[2].x = RAND_MAX;
	xyAxis4[2].y = 0.0f;
	xyAxis4[2].z = 0.0f;
	xyAxis4[2].r = 1.0f;
	xyAxis4[2].g = 1.0f;
	xyAxis4[2].b = 1.0f;

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	glPointSize(1.0);

	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
				0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				sizeof(vertex),     // stride
				(void*) offsetof(vertex, x) // array buffer offset
		);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
				1,                  // attribute. No particular reason for 0, but must match the layout in the shader.
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				sizeof(vertex),     // stride
				(void*) offsetof(vertex, r) // array buffer offset
		);

		glViewport(0, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
		glScissor(0, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
		glDrawArrays(GL_LINE_STRIP, xyAxis1_index, 3);
		glDrawArrays(GL_POINTS, 0, numOfRandomPointsPerGraph);

		glViewport(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
		glScissor(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
		glDrawArrays(GL_LINE_STRIP, xyAxis2_index, 3);
		glDrawArrays(GL_POINTS, g_vertex_buffer_data2_index, numOfRandomPointsPerGraph);

		glViewport(0, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
		glScissor(0, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
		glDrawArrays(GL_LINE_STRIP, xyAxis3_index, 3);
		glDrawArrays(GL_POINTS, g_vertex_buffer_data3_index, numOfRandomPointsPerGraph);

		glViewport(WINDOW_WIDTH / 2, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
		glScissor(WINDOW_WIDTH / 2, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
		glDrawArrays(GL_LINE_STRIP, xyAxis4_index, 3);
		glDrawArrays(GL_POINTS, g_vertex_buffer_data4_index, numOfRandomPointsPerGraph);


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
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
