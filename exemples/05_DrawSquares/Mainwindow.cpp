#include "MainWindow.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/gtc/matrix_transform.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

MainWindow::MainWindow()
{
}

int MainWindow::Initialisation()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();

	// Request OpenGL 4.6
	const char* glsl_version = "#version 460 core";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Draw squares", NULL, NULL);
	if (m_window == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(m_window);
	InitializeCallback();

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return 2;
	}

	// Other openGL initialization
	// -----------------------------
	return InitializeGL();
}

void MainWindow::InitializeCallback() {
	glfwSetWindowUserPointer(m_window, reinterpret_cast<void*>(this));
	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
		MainWindow* w = reinterpret_cast<MainWindow*>(glfwGetWindowUserPointer(window));
		w->FramebufferSizeCallback(width, height);
		});
	// For other callbacks:
	// https://www.glfw.org/docs/3.3/input_guide.html
}

int MainWindow::InitializeGL()
{
	// Setup framebuffer size (as we do additional work)
	FramebufferSizeCallback(SCR_WIDTH, SCR_HEIGHT);

	// Load and create shaders
	const std::string directory = SHADERS_DIR;
	bool mainShaderSuccess = true;
	m_mainShader = std::make_unique<ShaderProgram>();
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "triangles.vert");
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "triangles.frag");
	mainShaderSuccess &= m_mainShader->link();
	if (!mainShaderSuccess) {
		std::cerr << "Error when loading main shader\n";
		return 4;
	}

	// Vertex shader location
	int vPositionLocation;
	if ((vPositionLocation = m_mainShader->attributeLocation("vPosition")) < 0) {
		std::cerr << "Unable to find shader location for " << "vPosition" << std::endl;
		return 3;
	}

	// Check uniform attributes
	m_mainShader_matrix = m_mainShader->uniformLocation("uMatrix");
	m_mainShader_color = m_mainShader->uniformLocation("uColor");
	if (m_mainShader_matrix < 0 || m_mainShader_color < 0) {
		std::cerr << "Unable to find shader location for " << "uMatrix" << " or " << "uColor" << std::endl;
		return 3;
	}

	/////////////////////////
	// Geometry setup: separate vertices
	GLfloat VerticesSeparate[6][3] = {
		{ -0.4f, -0.4f, 0.0f }, // Triangle 1
		{  0.4f,  0.4f, 0.0f },
		{ -0.4f,  0.4f, 0.0f },
		{ -0.4f, -0.4f, 0.0f }, // Triangle 2
		{  0.4f, -0.4f, 0.0f },
		{  0.4f,  0.4f, 0.0f }
	};
	// Alternative: using indices 
	GLfloat VerticesShared[4][3] = {
		{ 0.4f,  0.4f, 0.0f },
		{-0.4f,  0.4f, 0.0f },
		{-0.4f, -0.4f, 0.0f },
		{  0.4f,-0.4f, 0.0f }
	};
	GLuint IndicesShared[2][3] = {
		{ 0, 1, 2 },
		{ 0, 2, 3 }
	};

	// Generate all buffers
	glCreateVertexArrays(NumVAOs, m_VAOs);
	glCreateBuffers(NumBuffers, m_buffers);

	// Upload vertex informations)
	glNamedBufferData(m_buffers[SeparateVertexBuffer], sizeof(VerticesSeparate), VerticesSeparate, GL_STATIC_DRAW);
	glNamedBufferData(m_buffers[SharedVertexBuffer], sizeof(VerticesShared), VerticesShared, GL_STATIC_DRAW);
	glNamedBufferData(m_buffers[SharedIndexBuffer], sizeof(IndicesShared), IndicesShared, GL_STATIC_DRAW);
	
	//////////////////////
	// Setup separate VAO
	glVertexArrayAttribFormat(m_VAOs[SeparateVAO], 
		vPositionLocation, // Attribute index 
		3, // Number of components
		GL_FLOAT, // Type 
		GL_FALSE, // Normalize 
		0 // Relative offset (first component)
	);
	glVertexArrayVertexBuffer(m_VAOs[SeparateVAO], 
		vPositionLocation, // Binding point 
		m_buffers[SeparateVertexBuffer], // VBO 
		0, // Offset (when the position starts)
		sizeof(glm::vec3) // Stride
	);
	glEnableVertexArrayAttrib(m_VAOs[SeparateVAO], 
		vPositionLocation // Attribute index
	);
	glVertexArrayAttribBinding(m_VAOs[SeparateVAO], 
		vPositionLocation, // Attribute index
		vPositionLocation  // Binding point
	);

	/////////////////////////
	// Setup shared index VAO
	glVertexArrayAttribFormat(m_VAOs[SharedVAO], 
		vPositionLocation, // Attribute index 
		3, // Number of components
		GL_FLOAT, // Type 
		GL_FALSE, // Normalize 
		0 // Relative offset (first component)
	);
	glVertexArrayVertexBuffer(m_VAOs[SharedVAO], 
		vPositionLocation, // Binding point 
		m_buffers[SharedVertexBuffer], // VBO 
		0, // Offset (when the position starts)
		sizeof(glm::vec3) // Stride
	);
	glEnableVertexArrayAttrib(m_VAOs[SharedVAO], 
		vPositionLocation // Attribute index
	);
	glVertexArrayAttribBinding(m_VAOs[SharedVAO], 
		vPositionLocation, // Attribute index
		vPositionLocation  // Binding point
	);
	// Elements (index)
	glVertexArrayElementBuffer(m_VAOs[SharedVAO], m_buffers[SharedIndexBuffer]);

	return 0;
}

void MainWindow::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_mainShader->bind();

	// Draw the first square (separate vertices)
	{
		glm::mat4 MvMatrix = glm::translate(glm::mat4(1), glm::vec3(-0.5, 0.5, 0.0));
		m_mainShader->setMat4(m_mainShader_matrix, MvMatrix);

		// red
		m_mainShader->setVec4(m_mainShader_color, glm::vec4(1.0, 0.0, 0.0, 1.0));

		// Bind VAO for separate vertices.
		// We use glDrawArray() to draw the shape.
		glBindVertexArray(m_VAOs[SeparateVAO]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	// Draw the second square (shared vertices)
	{
		glm::mat4 MvMatrix = glm::translate(glm::mat4(1), glm::vec3(0.5, 0.5, 0.0));
		m_mainShader->setMat4(m_mainShader_matrix, MvMatrix);

		// blue
		m_mainShader->setVec4(m_mainShader_color, glm::vec4(0.0, 0.0, 1.0, 1.0)); // rgba

		// Bind the VAO for the square with shared vertices.
		// Use glDrawElements() to draw triangles according to indices.
		glBindVertexArray(m_VAOs[SharedVAO]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
	}

	// Draw the third square (triangle fan)
	{
		glm::mat4 MvMatrix = glm::translate(glm::mat4(1), glm::vec3(0.0, -0.5, 0.0));
		m_mainShader->setMat4(m_mainShader_matrix, MvMatrix);

		// green
		m_mainShader->setVec4(m_mainShader_color, glm::vec4(0.0, 1.0, 0.0, 1.0)); // rgba

		// Here, we can re-use the shared vertex array.
		// But make a call to glDrawArrays()
		glBindVertexArray(m_VAOs[SharedVAO]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}
}

int MainWindow::RenderLoop()
{
	while (!glfwWindowShouldClose(m_window))
	{
		// Check inputs: Does ESC was pressed?
		if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(m_window, true);

		RenderScene();

		// Show rendering and get events
		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}

	// Cleanup
	glfwDestroyWindow(m_window);
	glfwTerminate();

	return 0;
}

void MainWindow::FramebufferSizeCallback(int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}