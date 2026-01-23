// ==========================================
// TODO: NEXT SESSION (The Chunk System)
// ==========================================
// 0. Fix the mouse sensitivity issue:
//    - Fix slow movement by keyboard and faster movement by mouse
// 
// 1. Create Chunk Class:
//    - Store 16x16x16 blocks (using a 3D array or flat vector)
//    - Generate a single Mesh for the whole chunk
//
// 2. Mesh Generation:
//    - Combine vertices of all blocks into one VBO
//    - (Optimization) "Face Culling": Don't draw faces that are touching other blocks
//
// 3. Performance:
//    - Draw 4096 blocks with just ONE glDrawArrays call
// ==========================================
#include <iostream>
#define TEST 0
#define DEBUG 0

#if TEST
#include <core/MathUtils.h>
#include <core/Ray.h>
#include <core/Matrix.h>
#include <core/camera.h>
#else
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <core/MathUtils.h>
#include <core/Matrix.h>
#include <core/camera.h>

#include <renderer/Shader.h>
#include <renderer/Buffer.h>
#include <renderer/VertexArray.h>
#include <renderer/Texture.h>

const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 720;

Core::Camera camera(Core::Vec3(0.0f, 0.0f, 3.0f));
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool bFirstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouseCallBack(GLFWwindow* pWindow, double xPosIn, double yPosIn) {
	float xPos = static_cast<float>(xPosIn);
	float yPos = static_cast<float>(yPosIn);

	if (bFirstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		bFirstMouse = false;
	}

	float xOffset = xPos - lastX;
	float yOffset = lastY - yPos; //Inverse Direction of rotate bottom to top

	lastX = xPos;
	lastY = yPos;

	camera.processMouseMovement(xOffset, yOffset);
}

void processInput(GLFWwindow* pWindow) {
	if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(pWindow, true);

	if (glfwGetKey(pWindow, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyboard(0, deltaTime);
	if (glfwGetKey(pWindow, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboard(1, deltaTime);
	if (glfwGetKey(pWindow, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard(2, deltaTime);
	if (glfwGetKey(pWindow, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard(3, deltaTime);
	if (glfwGetKey(pWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.processKeyboard(4, deltaTime);
	if (glfwGetKey(pWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.processKeyboard(5, deltaTime);
	if (glfwGetKey(pWindow, GLFW_KEY_R) == GLFW_PRESS) //Resets cameta position
	{
		camera.position = Core::Vec3(0.0f, 0.0f, 3.0f);
		camera.yaw = -90.0f;
		camera.pitch = 0.0f;
		camera.zoom = 45.0f;
		camera.front = Core::Vec3(0.0f, 0.0f, -1.0f);
		camera.updateCameraVectors();
	}
}

void scroll_callback(GLFWwindow* pWindow, double dXOffset, double dYOffset) {
	camera.processMouseScroll(static_cast<float> (dYOffset));
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
	glfwSetCursorPosCallback(pWindow, mouseCallBack);
	glfwSetScrollCallback(pWindow, scroll_callback);

	glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	Renderer::Shader shader("../assets/shaders/vertex.glsl", "../assets/shaders/fragment.glsl"); 

	/* for simple cube without textures
	float fVertices[] = {
		-0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f
	};*/
	float fVertices[] = {
		// positions          // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	Renderer::VertexArray vao;
	Renderer::VertexBuffer vbo(fVertices, sizeof(fVertices));

	vao.linkAttribute(vbo, 0, 3, 5, 0);
	vao.linkAttribute(vbo, 1, 2, 5, 3);
	Renderer::Texture texture("../assets/textures/container.jpg");
	texture.bind(0);

	lastFrame = static_cast<float>(glfwGetTime());
	while (!glfwWindowShouldClose(pWindow))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(pWindow);

		glClearColor(0.2f, 0.3f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();

		Core::Mat4 projection = Core::Mat4::perspective(camera.getZoom(),
			(float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 100.0f);
		Core::Mat4 view = camera.getViewMatrix();
		Core::Mat4 viewProjection = projection * view;
		shader.setMat4("uViewProjection", viewProjection);
		vao.bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);

#if DEBUG
		static float timer = 0.0f;
		timer += deltaTime;
		if (timer > 1.0f)
		{
			camera.position.print();
			timer = 0.0f;
		}
#endif

		glfwSwapBuffers(pWindow);
		glfwPollEvents();
	}
	glfwTerminate();
#endif
	return 0;
}
