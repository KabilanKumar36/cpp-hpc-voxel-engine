// ==========================================
// TODO: NEXT SESSION (Infinite Terrain & Noise)
// ==========================================
// 1. Chunk Coordinates:
//    - Update Chunk class to store its World Position (e.g., Chunk(0, 0), Chunk(1, 0))
//    - Offset vertex positions based on chunk coordinates so they don't overlap.
//
// 2. The World Manager:
//    - Create a `World` class to manage a std::vector or std::map of Chunks.
//    - Render a 4x4 grid of chunks (65,536 blocks!).
//
// 3. Procedural Generation:
//    - Integrate a Noise Library (FastNoiseLite or similar).
//    - Replace flat/solid fill with heightmap-based terrain (Hills & Valleys).
// ==========================================
#include <iostream>
#define TEST 0
#define DEBUG 0
#define BENCHMARK 0
#define SAMPLE_SINGLE_CUBE_TEST 0

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

#include <world/Chunk.h>

const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 720;
Core::Vec3 cameraPos = Core::Vec3(8.0f, 8.0f, 48.0f); //0.0f, 0.0f, 3.0f for Simple cube test
Core::Camera camera(cameraPos);
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool bFirstMouse = true;
bool bEnableFaceCulling = false;

float fDeltaTime = 0.0f;
float fLastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouseCallBack(GLFWwindow* pWindow, double xPosIn, double yPosIn) {
	bool bIsLeftDown = glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	bool bIsRightDown = glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
	if (!bIsLeftDown && !bIsRightDown)
	{
		bFirstMouse = true;
		return;
	}
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
	if(bIsLeftDown)
		camera.processMouseMovement(xOffset, yOffset);
	else if(bIsRightDown)
		camera.processMousePan(xOffset, yOffset);
}

void processInput(GLFWwindow* pWindow) {
	if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(pWindow, true);

	if (glfwGetKey(pWindow, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyboard(0, fDeltaTime);
	if (glfwGetKey(pWindow, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboard(1, fDeltaTime);
	if (glfwGetKey(pWindow, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard(2, fDeltaTime);
	if (glfwGetKey(pWindow, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard(3, fDeltaTime);
	if (glfwGetKey(pWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.processKeyboard(4, fDeltaTime);
	if (glfwGetKey(pWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.processKeyboard(5, fDeltaTime);
	if (glfwGetKey(pWindow, GLFW_KEY_R) == GLFW_PRESS) //Resets cameta position
	{
		camera.position = cameraPos;
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

void updateTitleInfo(GLFWwindow *pWindow, int iFrameCount) {
	if (!pWindow)
		return;
	std::string strTitle = "";
	if (bEnableFaceCulling)
	{
		strTitle = "HPC Voxel Engine FPS:" + std::to_string(iFrameCount) +
			"\tFace Culling Enabled (Press 'F' key to toogle)";
	}
	else
	{
		strTitle = "HPC Voxel Engine FPS:" + std::to_string(iFrameCount) +
			"\tFace Culling Disabled (Press 'F' key to toogle)";
	}
	glfwSetWindowTitle(pWindow, strTitle.c_str());
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
#if BENCHMARK
	glfwSwapInterval(0); // 0 = Unlock FPS (VSync OFF), 1 = Lock to 60 (VSync ON)
#endif
	glfwSetFramebufferSizeCallback(pWindow, framebuffer_size_callback);
	glfwSetCursorPosCallback(pWindow, mouseCallBack);
	glfwSetScrollCallback(pWindow, scroll_callback);

	//glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	const GLubyte* vendor = glGetString(GL_VENDOR);
	const GLubyte* renderor = glGetString(GL_RENDERER);
	std::cout << "GPU Renderer: " << vendor << std::endl;
	std::cout << "Renderer: " << renderor << std::endl;
	glEnable(GL_DEPTH_TEST);

	Renderer::Shader shader("../assets/shaders/vertex.glsl", "../assets/shaders/fragment.glsl"); 
	#if SAMPLE_SINGLE_CUBE_TEST
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
#else
	Chunk chunk;
	chunk.updateMesh();
#endif

	Renderer::Texture texture("../assets/textures/container.jpg");
	texture.bind(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	static bool sbFPressedLastTime = false;
	static int iFrameCount = 0;
	static float iTimer = 0.0f;
	std::string strFaceCullMsg = "";
	fLastFrame = static_cast<float>(glfwGetTime());
	while (!glfwWindowShouldClose(pWindow))
	{
		float fCurrentFrame = static_cast<float>(glfwGetTime());
		fDeltaTime = fCurrentFrame - fLastFrame;
		fLastFrame = fCurrentFrame;

		//For FPS Counter
		iFrameCount++;
		iTimer += fDeltaTime;

		if (glfwGetKey(pWindow, GLFW_KEY_F) == GLFW_PRESS)
		{
			if(!sbFPressedLastTime)
			{
				bEnableFaceCulling = !bEnableFaceCulling;
				sbFPressedLastTime = true;
				updateTitleInfo(pWindow, iFrameCount);
			}
		}
		else
			sbFPressedLastTime = false;
		if (iTimer >= 1.0f)
		{
			updateTitleInfo(pWindow, iFrameCount);
			iFrameCount = 0;
			iTimer = 0.0f;
		}

		processInput(pWindow);

		glClearColor(0.2f, 0.3f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.use();
		Core::Mat4 projection = Core::Mat4::perspective(camera.getZoom(),
			(float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 100.0f);
		Core::Mat4 view = camera.getViewMatrix();
		Core::Mat4 viewProjection = projection * view;
		shader.setMat4("uViewProjection", viewProjection);
#if SAMPLE_SINGLE_CUBE_TEST
		vao.bind();
		// for simple cube without textures
		glDrawArrays(GL_TRIANGLES, 0, 36);
#else
		if (chunk.getFaceCulling() != bEnableFaceCulling)
		{
			chunk.setFaceCulling(bEnableFaceCulling);
			chunk.updateMesh();
		}
		chunk.render();
#endif
#if DEBUG
		iTimer += fDeltaTime;
		if (iTimer > 1.0f)
		{
			camera.position.print();
			iTimer = 0.0f;
		}
#endif

		glfwSwapBuffers(pWindow);
		glfwPollEvents();
	}
	glfwTerminate();
#endif
	return 0;
}
