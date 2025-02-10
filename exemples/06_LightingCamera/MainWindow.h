#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/glm.hpp>

#include <iostream>
#include <vector>
#include <memory>

#include "ShaderProgram.h"


class MainWindow
{
public:
	MainWindow();

	// Main functions (initialization, run)
	int Initialisation();
	int RenderLoop();

	// Callback to intersept GLFW calls
	void FramebufferSizeCallback(int width, int height);

private:
	// Initialize GLFW callbacks
	void InitializeCallback();
	// Intiialize OpenGL objects (shaders, ...)
	int InitializeGL();

	// Rendering scene (OpenGL)
	void RenderScene();
	// Rendering interface ImGUI
	void RenderImgui();

	void updateCameraEye();

	void loadObjFile();

private:
	// GLFW Window
	GLFWwindow* m_window = nullptr;

	// settings
	const unsigned int SCR_WIDTH = 900;
	const unsigned int SCR_HEIGHT = 720;

	// Control de la camera
	float m_longitude = 0.0f ;
	const float m_minLongitude = -180.0f;
	const float m_maxLongitude = 180.0f;
	float m_latitude = 0.0f;
	const float m_minLatitude = -89;
	const float m_maxLatitude = 89;
	float m_distance = 8.0f;
	const float m_minDistance = 2;
	const float m_maxDistance = 14;

	// Camera related informations
	glm::vec3 m_eye, m_at, m_up;
	glm::mat4 m_proj;
	glm::vec3 m_light_position;

	// Main shader
	std::unique_ptr<ShaderProgram> m_mainShader = nullptr;
	struct m_mainShaderUniforms
	{
		GLint modelview; // mvMatrix
		GLint normal; // normalMatrix
		GLint proj; // projMatrix
		GLint lightPos; // lightPos
		GLint Kd; // Kd
		GLint Ks; // Ks
		GLint Kn; // Kn
	} m_mainShaderUniforms;

	// VAOs and VBOs
	struct MeshGL
	{
		// ID VAO/VBO
		GLuint vao;
		GLuint vboPosition;
		GLuint vboNormal;

		// Material information
		glm::vec3  diffuse;
		glm::vec3  specular;
		GLfloat    specularExponent;

		unsigned int numVertices;
	};
	std::vector<MeshGL> m_meshesGL;
};
