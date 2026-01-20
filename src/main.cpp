#include <iostream>
#define TEST 1
#if TEST
#include <core/MathUtils.h>
#include <core/Ray.h>
#else
#include <glad/glad.h>
#include <GLFW/glfw3.h>
const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 720;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}
#endif

int main() {
#if TEST
	std::cout << "Running in TEST mode" << std::endl;
	Core::Vec3 v1(1.0f, 2.0f, 3.0f);
	Core::Vec3 v2(4.0f, 5.0f, 6.0f);
	Core::Vec3 v3 = v1 + v2;
	Core::Vec3 v4 = v1.cross(v2);
	float dotProduct = v1.dot(v2);
	Core::Vec3 v5 = v1.normalize();
	std::cout << "v1 + v2 = " << v3 << std::endl;
	std::cout << "v1 x v2 = " << v4 << std::endl;
	std::cout << "v1 . v2 = " << dotProduct << std::endl;
	std::cout << "Normalized v1 = " << v5 << std::endl;

	Core::Vec3 forward(0.0f, 0.0f, -1.0f);
	Core::Vec3 up(0.0f, 1.0f, 0.0f);

	Core::Vec3 right = forward.cross(up).normalize();
	std::cout << "Right vector: " << right << std::endl;

	float alignment = forward.dot(up);
	std::cout << "Alignment between forward and up: " << alignment << std::endl;
	Core::Vec3 origin = Core::Vec3(0.0f, 0.0f, 0.0f);
	Core::Ray ray(origin, Core::Vec3(1.0f, 2.0f, 3.0f));
	Core::Vec3 pointAtT = ray.at(2.0f);
	std::cout << "Point along ray at t=2: " << pointAtT << std::endl;
#else
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* pWindow = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "HPC Voxel Engine", nullptr, nullptr);
	if (pWindow == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(pWindow);
	glfwSetFramebufferSizeCallback(pWindow, framebuffer_size_callback);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	while (!glfwWindowShouldClose(pWindow))
	{
		if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(pWindow, true);

		glClearColor(0.2f, 0.3f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(pWindow);
		glfwPollEvents();
	}
	glfwTerminate();
#endif
	return 0;
}
