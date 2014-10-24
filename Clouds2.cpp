// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <random>
#include <chrono>

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

int seed;
std::default_random_engine default_generator(std::chrono::system_clock::now().time_since_epoch().count());
std::knuth_b knuth_b_generator(std::chrono::system_clock::now().time_since_epoch().count());

struct vertex {
	GLfloat x, y, z;
	GLfloat r, g, b;
};

int current_window_width = WINDOW_WIDTH, current_window_height = WINDOW_HEIGHT;

void window_size_callback(GLFWwindow* window, int width, int height) {
	current_window_width = width;
	current_window_height = height;
}

/**
 * wrapper for rand(void)
 * min and max are inclusive
 */
int rand(int min, int max) {
	int range = max - min;
	return rand() % (range + 1) + min;
}

/**
 * seed must be initialized before use
 * min and max are inclusive
 */
int jsw_rand(int min, int max) {
	seed = A * ( seed % Q ) - R * ( seed / Q );
	if ( seed <= 0 )
		seed += M;
	//return seed % (max + 1);
	return min + ( seed % ( (max + 1) - min ) );
}

int uniform_int_distribution_rand(int min, int max) {
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(default_generator);
}

int knuth_b_rand(int min, int max) {
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(knuth_b_generator);
}

void generate_random_points(vertex* g_vertex_buffer_data, int length, int (*rand_function)(int, int)) {
	for (int i = 0; i < length; i++) {
		g_vertex_buffer_data[i].x = rand_function(0, RAND_MAX);
		g_vertex_buffer_data[i].y = rand_function(0, RAND_MAX);
		g_vertex_buffer_data[i].z = 0.0f;
		g_vertex_buffer_data[i].r = rand_function(0, 100) / 100.0f;
		g_vertex_buffer_data[i].g = rand_function(0, 100) / 100.0f;
		g_vertex_buffer_data[i].b = rand_function(0, 100) / 100.0f;
	}
}

void initialize_xy_axis(vertex* xyAxis) {
	xyAxis[0].x = 0.0f;
	xyAxis[0].y = RAND_MAX;
	xyAxis[0].z = 0.0f;
	xyAxis[0].r = 1.0f;
	xyAxis[0].g = 1.0f;
	xyAxis[0].b = 1.0f;

	xyAxis[1].x = 0.0f;
	xyAxis[1].y = 0.0f;
	xyAxis[1].z = 0.0f;
	xyAxis[1].r = 1.0f;
	xyAxis[1].g = 1.0f;
	xyAxis[1].b = 1.0f;

	xyAxis[2].x = RAND_MAX;
	xyAxis[2].y = 0.0f;
	xyAxis[2].z = 0.0f;
	xyAxis[2].r = 1.0f;
	xyAxis[2].g = 1.0f;
	xyAxis[2].b = 1.0f;
}

int main() {

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

	glfwSetWindowSizeCallback(window, window_size_callback);

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
			glm::vec3(17500, 16700, 44000), // Camera is at (4,3,3), in World Space
			glm::vec3(17500, 16700, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model      = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

	const int numOfRandomPointsPerGraph = 1000;
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

	//seed rand and jsw_rand
	srand(time(NULL)); //initialize random seed
	seed = time(NULL) % INT_MAX;

	//test random functions
	int (*functions[])(int, int) = {rand, jsw_rand, uniform_int_distribution_rand, knuth_b_rand};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 10; j++) {
			printf("%d\n", functions[i](15, 18));
		}
		printf("\n");
	}

	generate_random_points(g_vertex_buffer_data, numOfRandomPointsPerGraph, rand);
	initialize_xy_axis(xyAxis1);

	generate_random_points(g_vertex_buffer_data2, numOfRandomPointsPerGraph, jsw_rand);
	initialize_xy_axis(xyAxis2);

	generate_random_points(g_vertex_buffer_data3, numOfRandomPointsPerGraph, uniform_int_distribution_rand);
	initialize_xy_axis(xyAxis3);

	generate_random_points(g_vertex_buffer_data4, numOfRandomPointsPerGraph, knuth_b_rand);
	initialize_xy_axis(xyAxis4);

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

		if (glfwGetKey( window, GLFW_KEY_1 ) == GLFW_PRESS){
			generate_random_points(g_vertex_buffer_data, numOfRandomPointsPerGraph, rand);
			glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
		}
		if (glfwGetKey( window, GLFW_KEY_2 ) == GLFW_PRESS){
			generate_random_points(g_vertex_buffer_data2, numOfRandomPointsPerGraph, rand);
			glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
		}
		if (glfwGetKey( window, GLFW_KEY_3 ) == GLFW_PRESS){
			generate_random_points(g_vertex_buffer_data3, numOfRandomPointsPerGraph, rand);
			glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
		}
		if (glfwGetKey( window, GLFW_KEY_4 ) == GLFW_PRESS){
			generate_random_points(g_vertex_buffer_data4, numOfRandomPointsPerGraph, rand);
			glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
		}

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

		glViewport(0, current_window_height / 2, current_window_width / 2, current_window_height / 2);//upper left
		glScissor(0, current_window_height / 2, current_window_width / 2, current_window_height / 2);
		glDrawArrays(GL_LINE_STRIP, xyAxis1_index, 3);
		glDrawArrays(GL_POINTS, 0, numOfRandomPointsPerGraph);

		glViewport(current_window_width / 2, current_window_height / 2, current_window_width / 2, current_window_height / 2);//upper right
		glScissor(current_window_width / 2, current_window_height / 2, current_window_width / 2, current_window_height / 2);
		glDrawArrays(GL_LINE_STRIP, xyAxis2_index, 3);
		glDrawArrays(GL_POINTS, g_vertex_buffer_data2_index, numOfRandomPointsPerGraph);

		glViewport(0, 0, current_window_width / 2, current_window_height / 2);//lower left
		glScissor(0, 0, current_window_width / 2, current_window_height / 2);
		glDrawArrays(GL_LINE_STRIP, xyAxis3_index, 3);
		glDrawArrays(GL_POINTS, g_vertex_buffer_data3_index, numOfRandomPointsPerGraph);

		glViewport(current_window_width / 2, 0, current_window_width / 2, current_window_height / 2);//lower right
		glScissor(current_window_width / 2, 0, current_window_width / 2, current_window_height / 2);
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
